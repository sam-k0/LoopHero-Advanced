#pragma once
#include <fstream>
#include <iterator>
#include <string>
#include <Windows.h>
#include <vector>
#include <direct.h>
#include <ostream>
#include <sstream>
#include <iomanip>
//#define DEBUG // enables more prints
#include "SDK/SDK.hpp"
#include "LH/Filesystem.h"

HINSTANCE DllHandle; 

static std::string gPluginName = "sam-k0.Advanced.yytk";
static std::string cfgFilename = Filesys::GetCurrentDir() + "\\options.ini";
static std::string SectionName = "Advanced"; 
static std::string SettingKeyModEnabled = "Enabled";

DllExport std::string GetPluginName() // For yytk, mandatory
{
    return gPluginName;
}

// Some useful functions that don't fit into their own category (may be restructured in the future)
namespace Misc {
    // Never pass text as the format string, game strings contain '%' ("% chance for").
    void Print(std::string s, Color c = CLR_DEFAULT)
    {
        PrintMessage(c, "%s", (gPluginName + ": " + s).c_str());
    }

    void PrintDbg(std::string s, std::string func, int line, Color c)
    {
        PrintMessage(c, "%s", (gPluginName + ": " + s + " (" + func + ":" + std::to_string(line) + ")").c_str());
    }

    // checks if a string s contains a substring subs
    bool StringHasSubstr(std::string s, std::string subs)
    {
        if (s.find(subs) != std::string::npos) {
            return true;
        }
        return false;
    }

    //https://www.techiedelight.com/check-if-a-string-ends-with-another-string-in-cpp/
    bool StringEndsWith(std::string const& str, std::string const& suffix) {
        if (str.length() < suffix.length()) {
            return false;
        }
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    }
}

namespace Binds {
    void GetInstanceVariables(YYRValue& arr, YYRValue inst)
    {
        CallBuiltin(arr, "variable_instance_get_names", nullptr, nullptr, { inst });
    }

    YYRValue CallBuiltin(const std::string name, CInstance* self, CInstance* other, const std::vector<YYRValue> args)
    {
        YYRValue var;
        CallBuiltin(var, name, self, other, args);
        return var;
    }

    YYRValue CallBuiltinA(const std::string name, const std::vector<YYRValue> args)
    {
        YYRValue var;
        CallBuiltin(var, name, nullptr, nullptr, args);
        return var;
    }

    YYRValue GetVariable(YYRValue inst, const char* varname)
    {
        YYRValue var;
        CallBuiltin(var, "variable_instance_get", nullptr, nullptr, { inst, varname });
        return var;
	}

    void SetVariable(YYRValue inst, const char* varname, YYRValue value)
    {
        CallBuiltin(value, "variable_instance_set", nullptr, nullptr, { inst, varname, value });
	}

    void PrintArrayInstanceVariables(YYRValue var, YYRValue inst, Color c = Color::CLR_DEFAULT)
    {
        YYRValue len;
        YYRValue item;
        YYRValue content;
        YYRValue type;
        CallBuiltin(len, "array_length_1d", nullptr, nullptr, { var });

        for (int i = 0; i < (int)len; i++)
        {
            CallBuiltin(item, "array_get", nullptr, nullptr, { var, (double)i });
            std::string varname = item.operator std::string();
            if (varname.empty())
                continue;
            CallBuiltin(content, "variable_instance_get", nullptr, nullptr, { inst, varname });
            CallBuiltin(type, "typeof", nullptr, nullptr, { content });
            std::string typestr = type.operator std::string();
            std::string message = varname + " -> " + typestr;

            if (typestr == "number")
            {
                message += " : " + std::to_string((int)content);
            }
            else if (typestr == "bool")
            {
                message += " : " + std::to_string(int((bool)content));
            }
            else if (typestr == "string")
            {
                message += " : " + content.operator std::string();
            }

            Misc::Print(message, c);
        }
    }

}