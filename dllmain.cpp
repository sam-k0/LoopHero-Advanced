#include <Windows.h>
#include <fstream>
#include <iterator>
#include <format>
#include <algorithm>
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
//double g_newLangButtonRef;

// Polish lang
double g_LanguageButtonPolishRef;
double g_LanguageButtonPolishSpriteRef;
const char* g_LanguageButtonPolishLangId = "pol";
// Vietnamese lang
double g_LanguageButtonVietnameseRef;
double g_LanguageButtonVietnameseSpriteRef;
const char* g_LanguageButtonVietnameseLangId = "vie";
// Thai lang
double g_LanguageButtonThaiRef;
double g_LanguageButtonThaiSpriteRef;
const char* g_LanguageButtonThaiLangId = "tha";


double CreateLanguageButton(const char* langId, double x, double y, double spriteRef)
{
    double buttonRef = Binds::CallBuiltinA("instance_create_depth", {x, y, -14002., (double)LHObjectEnum::o_opt_lang_button });
    Binds::SetVariable(buttonRef, "language", langId);
    Binds::SetVariable(buttonRef, "text_lang", langId);
    Binds::SetVariable(buttonRef, "xx", x + 218.);
    Binds::SetVariable(buttonRef, "sprite_index", spriteRef);
    return buttonRef;
}

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
    /*if (strcmp(codeObj->i_pName, "gml_Object_o_base_button_Mouse_4") == 0 && int(g_newLangButtonRef) == selfInst->i_id)
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
    }*/

    // Dont call step event on custom buttons cause it will break
    /*if (strcmp(codeObj->i_pName, "gml_Object_o_opt_lang_button_Step_2") == 0)
    {
        //return YYTK_DONTCALL; // maybe its fixed by setting xx
        //&& int(g_LanguageButtonPolishRef) == selfInst->i_id

		std::vector<int> langButtonIds = {(int)g_LanguageButtonPolishRef, (int)g_LanguageButtonVietnameseRef, (int)g_LanguageButtonThaiRef };
		if (std::find(langButtonIds.begin(), langButtonIds.end(), selfInst->i_id) != langButtonIds.end())
        {
            return YYTK_DONTCALL;
        }
    }*/


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

            /*g_newLangButtonRef = static_cast<double>(Binds::CallBuiltinA("instance_create_depth", {531., 90., -10010., (double)LHObjectEnum::o_menu_button}));
            Binds::CallBuiltinA("variable_instance_set", { g_newLangButtonRef, "click_event", -1. }); // delete original callback
            Binds::CallBuiltinA("variable_instance_set", { g_newLangButtonRef, "text", "test" });
            Binds::CallBuiltinA("variable_instance_set", { g_newLangButtonRef, "fa_ltext", "test" });
            */
            // new real lang buton
            const char* langid = "pol";
            // offsets are x 29, y 19
            g_LanguageButtonPolishRef = CreateLanguageButton(langid, 548., 40., g_LanguageButtonPolishSpriteRef);			
			g_LanguageButtonThaiRef = CreateLanguageButton(g_LanguageButtonThaiLangId, 548.-29.,40., g_LanguageButtonThaiSpriteRef);
			g_LanguageButtonVietnameseRef = CreateLanguageButton(g_LanguageButtonVietnameseLangId, 548.-58., 40., g_LanguageButtonVietnameseSpriteRef);
        }

    }

    if (std::string(codeObj->i_pName).starts_with("gml_Room_"))
    {
		g_CreatedlangButtons = false; // reset this so lang buttons can be spawned again in the next room
    }

    return YYTK_OK;
}

void InstallPatches()
{
    // Require dependencies janky as hell
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

    // load assets
	g_LanguageButtonPolishSpriteRef = Assets::AddSprite("Advanced\\flag_pol.png", 3, true, false, 0, 0);
    g_LanguageButtonThaiSpriteRef = Assets::AddSprite("Advanced\\flag_tha.png", 3, true, false, 0, 0);
    g_LanguageButtonVietnameseSpriteRef = Assets::AddSprite("Advanced\\flag_vie.png", 3, true, false, 0, 0);

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

