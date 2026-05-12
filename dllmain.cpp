#include <Windows.h>
#include <fstream>
#include <iterator>

#include "PluginSetup.h"
#include "LH/Assets.h"
#include "LH/LHSprites.h"
#include "LH/LHCore.h" // mandatory core functions
#include "LH/Config.h" // ini config
#include "LH/CallbackCore.h"

static float MyValue = 8.0; // some random default value to save and load


// Unload function, remove callbacks here
YYTKStatus PluginUnload()
{
    LHCore::pUnregisterModule(gPluginName);
    return YYTK_OK;
}

int CodePostPatch(YYTKCodeEvent* codeEvent, void* p_rawCCAttr)
{
    
    CCode* codeObj = std::get<CCode*>(codeEvent->Arguments());
    CInstance* selfInst = std::get<0>(codeEvent->Arguments());
    CInstance* otherInst = std::get<1>(codeEvent->Arguments());

    // If we have invalid data???
    if (!codeObj)
        return YYTK_INVALIDARG;

    if (!codeObj->i_pName)
        return YYTK_INVALIDARG;

    auto* ccAttr = static_cast<CallbackCoreAttributes*>(p_rawCCAttr);

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



void InstallPatches() // Register Pre and Post patches here
{
	if (LHCore::pInstallPostPatch != nullptr && LHCore::pInstallPrePatch != nullptr)
	{
		LHCore::pInstallPostPatch(CodePostPatch); // Method will run after CodeExecute
        //LHCore::pInstallPrePatch(CodePrePatch); // Method will run before CodeExecute, but dont do the same function for both... cause why would you?
        Misc::Print("Installed patch method(s)", CLR_GREEN);
	}


    // All things related to config file. If your plugin doesn't have any settings, you can safely remove this chunk.
    if (Filesys::FileExists(cfgFilename))
    {
        if (Config::KeySectionExists(cfgFilename, SectionName, KeyName)) {
            // Read the value
            Misc::PrintDbg("Reading config values", __FUNCTION__, __LINE__, CLR_GREEN);
            MyValue = float(Config::ReadIntFromIni(cfgFilename, SectionName, KeyName, 8));
        }
        else
        {
            // Write a default value
            Misc::PrintDbg("Writing default values to config", __FUNCTION__, __LINE__, CLR_GRAY);
            Config::WriteIniValue(cfgFilename, SectionName, KeyName, std::to_string(8.0));
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

