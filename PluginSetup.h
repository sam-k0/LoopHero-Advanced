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

inline HINSTANCE DllHandle;

static std::string gPluginName = "sam-k0.loophero.advanced";
static std::string cfgFilename = Filesys::GetCurrentDir() + "\\options.ini";
static std::string SectionName = "Advanced"; 
static std::string SettingKeyModEnabled = "Enabled";

DllExport inline std::string GetPluginName() // For yytk, mandatory
{
    return gPluginName;
}

// Some useful functions that don't fit into their own category (may be restructured in the future)
namespace Misc {
    // Never pass text as the format string, game strings contain '%' ("% chance for").
    inline void Print(std::string s, Color c = CLR_DEFAULT)
    {
        PrintMessage(c, "%s", (gPluginName + ": " + s).c_str());
    }

    inline void PrintDbg(std::string s, std::string func, int line, Color c)
    {
        PrintMessage(c, "%s", (gPluginName + ": " + s + " (" + func + ":" + std::to_string(line) + ")").c_str());
    }

    // checks if a string s contains a substring subs
    inline bool StringHasSubstr(std::string s, std::string subs)
    {
        if (s.find(subs) != std::string::npos) {
            return true;
        }
        return false;
    }

    //https://www.techiedelight.com/check-if-a-string-ends-with-another-string-in-cpp/
    inline bool StringEndsWith(std::string const& str, std::string const& suffix) {
        if (str.length() < suffix.length()) {
            return false;
        }
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    }
}

namespace Binds {
    inline YYRValue CallBuiltin(const std::string name, CInstance* self, CInstance* other, const std::vector<YYRValue> args)
    {
        YYRValue var;
        CallBuiltin(var, name, self, other, args);
        return var;
    }

    inline YYRValue CallBuiltinA(const std::string name, const std::vector<YYRValue> args)
    {
        YYRValue var;
        CallBuiltin(var, name, nullptr, nullptr, args);
        return var;
    }

    inline YYRValue GetVariable(YYRValue inst, const char* varname)
    {
        YYRValue var;
        CallBuiltin(var, "variable_instance_get", nullptr, nullptr, { inst, varname });
        return var;
	}

    inline void SetVariable(YYRValue inst, const char* varname, YYRValue value)
    {
        CallBuiltin(value, "variable_instance_set", nullptr, nullptr, { inst, varname, value });
	}
}