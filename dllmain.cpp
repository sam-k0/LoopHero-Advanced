#include <Windows.h>
#include <fstream>
#include <iterator>
#include <format>
#include "PluginSetup.h"
#include "LH/Assets.h"
#include "LH/LHSprites.h"
#include "LH/LHCore.h" // mandatory core functions
#include "LH/Config.h" // ini config
#include "LH/CallbackCore.h"
#include "Dependencies.h"

static float SettingValueModEnabled = 1.0; // Mod enabled


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

    /*
    * Do your things here. This is probably the most interesting part of the code
    * You can change game behavior here, get the calling object's information and much more.
    * ---------------
    * codeObj contains information about the event.
    * selfInst is the calling instance.
    * otherInst is the other instance, for example in collision events.
    */

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

    if (ccAttr->call == OriginalCall::CANCELLED) // If you only want to run this post-patch when the original code was run
    {
        Misc::Print("Error: this plugin needs the original event to be called!");
        return YYTK_OK; 
    }

    // Print all args
    if (strcmp(codeObj->i_pName, "gml_Object_o_menu_button_Alarm_6") == 0) // Only print args for "with" events, otherwise it gets spammy
    {
        Misc::Print("Yep");
        auto& args = codeEvent->Arguments();

        std::apply([](auto&&... vals)
            {
                ((Misc::Print(std::format("Arg: {}", (void*)vals))), ...);
            }, args);
    }


    /* 
    * Do your things here. This is probably the most interesting part of the code
    * You can change game behavior here, get the calling object's information and much more.
    * ---------------
    * codeObj contains information about the event.
    * selfInst is the calling instance.
    * otherInst is the other instance, for example in collision events.
    */

    return YYTK_OK;
}

void InstallPatches()
{
    // Require dependencies
    // Janky as hell. I never thought about priority management when designing the core.
    for (int i = 0; i < 10; i++) { 
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


	if (LHCore::pInstallPostPatch != nullptr && LHCore::pInstallPrePatch != nullptr)
	{
		LHCore::pInstallPostPatch(CodePostPatch); // Method will run after CodeExecute
        LHCore::pInstallPrePatch(CodePrePatch); // Method will run before CodeExecute, but dont do the same function for both... cause why would you?
        Misc::Print("Installed patch method(s)", CLR_GREEN);
	}


    if (Filesys::FileExists(cfgFilename))
    {
        if (Config::KeySectionExists(cfgFilename, SectionName, SettingKeyModEnabled)) {
            // Read the value
            SettingValueModEnabled = float(Config::ReadIntFromIni(cfgFilename, SectionName, SettingKeyModEnabled, SettingValueModEnabled));
            if (SettingValueModEnabled != 0.0f)
            {
                PluginUnload();
            }
        }
        else
        {
            // Write a default value
            Config::WriteIniValue(cfgFilename, SectionName, SettingKeyModEnabled, std::to_string(SettingValueModEnabled));
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

