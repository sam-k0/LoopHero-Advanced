#include <Windows.h>
#include <fstream>
#include <iterator>
#include <format>
#include "PluginSetup.h"
#include "LH/Assets.h"
#include "LH/LHSprites.h"
#include "LH/LHObjects.h"
#include "LH/LHCore.h" // mandatory core functions
#include "LH/Config.h" // ini config
#include "LH/CallbackCore.h"
#include "Dependencies.h"

static float g_SettingValueModEnabled = 1.0; // Mod enabled

static bool g_CreatedlangButtons = false;
double g_OrigLangButtonRef;
double g_newLangButtonRef;
double g_LanguageButtonCopyRef;

// Unload function, remove callbacks here
YYTKStatus PluginUnload()
{
    LHCore::pUnregisterModule(gPluginName);
    return YYTK_OK;
}

int CodePrePatch(YYTKCodeEvent* codeEvent, void* p_rawCCAttr)
{

    CCode* codeObj = std::get<CCode*>(codeEvent->Arguments());
    CInstance* selfInst = std::get<0>(codeEvent->Arguments());
    CInstance* otherInst = std::get<1>(codeEvent->Arguments());
    CallbackCoreAttributes* ccAttr = static_cast<CallbackCoreAttributes*>(p_rawCCAttr);

    // If we have invalid data???
    if (!codeObj)
        return YYTK_INVALIDARG;

    if (!codeObj->i_pName)
        return YYTK_INVALIDARG;

    if (ccAttr->call == OriginalCall::CANCELLED) // If you only want to run this post-patch when the original code was run
    {
        Misc::Print("Error: this plugin needs the original event to be called!");
        return YYTK_OK;
    }

    // new lang button catched
    if (strcmp(codeObj->i_pName, "gml_Object_o_base_button_Mouse_4") == 0 && int(g_newLangButtonRef) == selfInst->i_id)
    {
        // then catch the button press of that button, set the global variable "lang" to the correct lang id, and emit the "other_10" event on a language button
        // or alternatively set the language of the english button to custom, and then trigger the other 10 event
        // and then set the language back to english...
		Misc::Print("this is doing nothing for now.");

        return YYTK_DONTCALL;
    }

    if (strcmp(codeObj->i_pName, "gml_Object_o_menu_button_Alarm_6") == 0 && int(g_newLangButtonRef) == selfInst->i_id)
    {
        return YYTK_DONTCALL;
    }

    // Dont call step event on custom buttons cause it will break
    if (strcmp(codeObj->i_pName, "gml_Object_o_opt_lang_button_Step_2") == 0 && int(g_LanguageButtonCopyRef) == selfInst->i_id)
    {
        //return YYTK_DONTCALL;
    }


    return YYTK_OK;
}

int CodePostPatch(YYTKCodeEvent* codeEvent, void* p_rawCCAttr)
{
    
    CCode* codeObj = std::get<CCode*>(codeEvent->Arguments());
    CInstance* selfInst = std::get<0>(codeEvent->Arguments());
    CInstance* otherInst = std::get<1>(codeEvent->Arguments());
    CallbackCoreAttributes* ccAttr = static_cast<CallbackCoreAttributes*>(p_rawCCAttr);

    // If we have invalid data???
    if (!codeObj)
        return YYTK_INVALIDARG;

    if (!codeObj->i_pName)
        return YYTK_INVALIDARG;


    if (strcmp(codeObj->i_pName, "gml_Object_o_opt_lang_button_Create_0") == 0)
    {

        if (!g_CreatedlangButtons)
        {
            g_CreatedlangButtons = true; // set this to false upon room change to spawn buttons again...
            g_OrigLangButtonRef = double(selfInst->i_id);
            // Create another button manually and set sprite

            g_newLangButtonRef = static_cast<double>(Binds::CallBuiltinA("instance_create_depth", { 531., 90., -10010., (double)LHObjectEnum::o_menu_button }));
            Binds::CallBuiltinA("variable_instance_set", { g_newLangButtonRef, "click_event", -1. }); // delete original callback
            Binds::CallBuiltinA("variable_instance_set", { g_newLangButtonRef, "text", "test" });
            Binds::CallBuiltinA("variable_instance_set", { g_newLangButtonRef, "fa_ltext", "test" });

            // new real lang buton
            const char* langid = "pol";
            g_LanguageButtonCopyRef = Binds::CallBuiltinA("instance_create_depth", {548.,40., -14002., (double)LHObjectEnum::o_opt_lang_button });
            Binds::SetVariable(g_LanguageButtonCopyRef, "language", langid);
            Binds::SetVariable(g_LanguageButtonCopyRef, "text_lang", langid);
            Binds::SetVariable(g_LanguageButtonCopyRef, "xx", 548+218.);
        }

    }

    return YYTK_OK;
}

void InstallPatches()
{
    // Require dependencies
    // Janky as hell. I never thought about priority management when designing the core.
    /*for (int i = 0; i < 10; i++) {
        if (!Deps::RequireDependency("sam-k0.SnowStorm.yytk"))
        {
            Misc::Print("sam-k0.SnowStorm.yytk is a required depedency, Waiting...", CLR_RED);
            if (i == 9)
            {   
				Misc::Print("Failed to load required dependencies, unloading plugin", CLR_RED);
                PluginUnload();
            }
            return;
        }
		Sleep(100);
    }
   
    // Import functions from dependencies
    */

	if (LHCore::pInstallPostPatch != nullptr && LHCore::pInstallPrePatch != nullptr)
	{
		LHCore::pInstallPostPatch(CodePostPatch);
        LHCore::pInstallPrePatch(CodePrePatch);
        Misc::Print("Installed patch method(s)", CLR_GREEN);
	}


    if (Filesys::FileExists(cfgFilename))
    {
        if (Config::KeySectionExists(cfgFilename, SectionName, SettingKeyModEnabled)) {
            // Read the value
            g_SettingValueModEnabled = Config::ReadIntFromIni(cfgFilename, SectionName, SettingKeyModEnabled, g_SettingValueModEnabled);
            if (g_SettingValueModEnabled == 0)
            {
                Misc::Print("Disabled in options.ini, bye", CLR_RED);
                PluginUnload();
            }
        }
        else
        {
            // Write a default value
            Config::WriteIniValue(cfgFilename, SectionName, SettingKeyModEnabled, std::to_string(g_SettingValueModEnabled));
        }
    }

}

// Entry
DllExport YYTKStatus PluginEntry(
    YYTKPlugin* PluginObject // A pointer to the dedicated plugin object
)
{
    LHCore::CoreReadyPack* pack = new LHCore::CoreReadyPack(PluginObject, InstallPatches); // InstallPatches will be ran as soon as CallbackCore is ready.
    PluginObject->PluginUnload = PluginUnload;
    CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LHCore::ResolveCore, (LPVOID)pack, 0, NULL)); // Wait for LHCC
    return YYTK_OK; // Successful PluginEntry.
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DllHandle = hModule; // save our module handle
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

