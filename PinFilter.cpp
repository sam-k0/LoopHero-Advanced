#include "PinFilter.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include "LH/Config.h"
#include "LH/Filesystem.h"
#include "LH/LHDraw.h"
#include "LH/LHObjects.h"
#include "LH/LHSprites.h"

namespace PinFilter {

    // Indexed by itm_index, same order as the s_equipslot_empty frames (frame = itm_index + 1). Index 5 is a small
    // ring silhouette without a lang name that no class has a slot for.
    static const char* TypeKeys[] = { "srddef", "hlmdef", "shlddef", "hnddef", "armrdef", nullptr, "amldef", "bltdef", "rngdef", "btsdef", "stfdef" };
    static constexpr int TypeCount = sizeof(TypeKeys) / sizeof(TypeKeys[0]);
    static std::string g_TypeLabels[TypeCount];
    static std::string g_LabelsLang;

    static const std::string SectionName = "PinFilter";

    static bool g_TypeEnabled[TypeCount] = {};
    static std::unordered_set<int> g_NewItems;
    static std::vector<int> g_VisibleTypes; // menu row -> item type
    static int g_DropListSize = -1; // when g_VisibleTypes was built

    // Square o_text_button in the free top right cell of the equipment grid, with an icon instead of text
    static double g_ButtonX = 0.;
    static double g_ButtonY = 0.;
    static bool g_ButtonShown = false;
    static double g_ButtonRef = -1.;
    static bool g_ButtonCreated = false; // once per room, even if creating failed
    static bool g_CreatingButton = false; // its Create event runs inside instance_create_depth
    static double g_FreeCellX = 608.;
    static double g_FreeCellY = 27.;
    static const char* MenuTitle = "Filter";
    static constexpr double ButtonSize = 25.;      // s_expsquarebutton, origin -1,0, frames normal/hover/pressed
    // 2x2 grid icon baked into the top left of s_equipmenu (camp equipment screen) at 8,6, 13x13 strokes on plain
    // stone (104,111,111). Cropped one pixel larger to the left and bottom, so the bottom left pixel is stone and
    // sprite_create_from_surface(removeback) cuts the icon out.
    static constexpr double IconSourceX = 7.;
    static constexpr double IconSourceY = 6.;
    static constexpr double IconCropSize = 14.;
    static constexpr double IconSize = 13.;
    static double g_IconSprite = -1.;
    static constexpr double HiddenPos = -1000.; // invisible instances still get mouse events
    static constexpr double RoomWidth = 640.;
    static constexpr double RoomHeight = 360.;
    static bool g_MenuOpen = false;
    static bool g_MouseOverUI = false; // from the last drawn frame

    // Menu panel built from the s_dead_menu_* sprites, one native o_opt_button checkbox per item type
    static constexpr double MenuWidth = 141.;
    static constexpr double MenuHeaderHeight = 63.; // s_dead_menu_toplut, origin at its bottom left
    static constexpr double MenuPlateHeight = 32.;  // stone plate at the bottom of s_dead_menu_toplut
    static constexpr double MenuMidHeight = 27.;    // s_dead_menu_midlut, stretched vertically
    static constexpr double MenuBottomHeight = 5.;  // s_dead_menu_bot
    static constexpr double MenuPadding = 6.;
    static constexpr double CheckboxSize = 15.;     // s_opt_shriftgalki, frame = checked * 3 + hover + pressed
    static constexpr double CheckboxDepth = -15001.;
    static constexpr double RowHeight = 16.;
    static std::vector<double> g_Checkboxes;
    static bool g_CheckboxWasPressed[TypeCount] = {};
    static double g_MenuX = 0.;
    static double g_MenuContentY = -1.;
    static bool g_MenuBackgroundDrawn = false;

    static constexpr double ColorText = 0xE6E6E6; // BGR
    static constexpr double ColorTextEnabled = 0x5ABEEB;
    static constexpr double ColorTitle = 0x2A2A2A;

    void LoadConfig()
    {
        for (int i = 0; i < TypeCount; i++)
            g_TypeEnabled[i] = TypeKeys[i] && Config::ReadIntFromIni(cfgFilename, SectionName, TypeKeys[i], 0) != 0;
    }

    void RefreshTypeLabels()
    {
        std::string lang = Binds::CallBuiltinA("variable_global_get", { "lang" });
        if (lang == g_LabelsLang && !g_LabelsLang.empty())
            return;
        g_LabelsLang = lang;

        std::string langFile = Filesys::GetCurrentDir() + "\\local\\lang_" + lang + ".ini";
        if (lang.empty() || !Filesys::FileExists(langFile))
            langFile = Filesys::GetCurrentDir() + "\\local\\lang_eng.ini";

        for (int i = 0; i < TypeCount; i++)
        {
            if (TypeKeys[i])
                g_TypeLabels[i] = Config::ReadIniValue(langFile, "itm_name", TypeKeys[i], TypeKeys[i]);
        }
    }

    // Raw values of a global ds_list, empty if it doesn't exist
    std::vector<int> ReadGlobalList(const char* name)
    {
        std::vector<int> values;
        double list = Binds::CallBuiltinA("variable_global_get", { name });
        if (!(bool)Binds::CallBuiltinA("ds_exists", { list, 2. })) // ds_type_list
            return values;
        int size = (int)(double)Binds::CallBuiltinA("ds_list_size", { list });
        for (int i = 0; i < size; i++)
            values.push_back((int)(double)Binds::CallBuiltinA("ds_list_find_value", { list, (double)i }));
        return values;
    }

    // Cheap per frame check if a slot got unlocked while the menu is open
    int DropListSize()
    {
        double list = Binds::CallBuiltinA("variable_global_get", { "class_droplist" });
        if (!(bool)Binds::CallBuiltinA("ds_exists", { list, 2. })) // ds_type_list
            return 0;
        return (int)(double)Binds::CallBuiltinA("ds_list_size", { list });
    }

    // global.class_droplist holds the itm_index values __item_generate picks from. It starts as the types of the
    // class' equipment grid (global.class_slotlist, 3x4 row major, -1 = no slot) and gains a type when a slot is
    // unlocked during the expedition (e.g. the Arsenal card adds the amulet for the rogue), class_slotlist doesn't.
    // Falls back to class_slotlist, then to all types.
    void RefreshVisibleTypes()
    {
        std::vector<int> drops = ReadGlobalList("class_droplist");
        std::vector<int> slots = ReadGlobalList("class_slotlist");
        g_DropListSize = (int)drops.size();

        bool hasSlot[TypeCount] = {};
        bool any = false;
        for (int type : drops.empty() ? slots : drops)
        {
            if (type >= 0 && type < TypeCount && TypeKeys[type])
                hasSlot[type] = any = true;
        }

        g_VisibleTypes.clear();
        for (int i = 0; i < TypeCount; i++)
        {
            if (TypeKeys[i] && (!any || hasSlot[i]))
                g_VisibleTypes.push_back(i);
        }
    }

    void SetTypeEnabled(int type, bool enabled)
    {
        g_TypeEnabled[type] = enabled;
        Config::WriteIniValue(cfgFilename, SectionName, TypeKeys[type], enabled ? "1" : "0");
    }

    // Same as o_item_Mouse_5 when it locks an item
    void Pin(double item)
    {
        double list = Binds::CallBuiltinA("variable_global_get", { "inv_items" });
        double pos = Binds::CallBuiltinA("ds_list_find_index", { list, item });
        if (pos < 0)
            return;

        Binds::CallBuiltinA("ds_list_delete", { list, pos });
        Binds::CallBuiltinA("ds_list_insert", { list, 0., item });
        Binds::SetVariable(item, "inv_lock", 1.);
    }

    // Post gml_Object_o_item_Alarm_0
    void OnItemGenerated(CInstance* item)
    {
        // Alarm_0 only re-arms itself while the item is delayed, the stats aren't rolled yet
        if ((double)Binds::GetVariable(double(item->i_id), "delay") > 0)
            return;
        g_NewItems.insert(item->i_id);
    }

    // Post gml_Object_o_item_Alarm_1. Only fresh drops are in g_NewItems, items loaded from a save stay as they were.
    void OnItemPlaced(CInstance* itemInst)
    {
        if (g_NewItems.erase(itemInst->i_id) == 0)
            return;

        double item = double(itemInst->i_id);
        int type = (int)Binds::GetVariable(item, "itm_index");
        if (type < 0 || type >= TypeCount || !TypeKeys[type])
            return;

        bool pin = g_TypeEnabled[type]
            && (double)Binds::GetVariable(item, "equip_slot") == -1.
            && !(bool)Binds::GetVariable(item, "inv_lock");
        if (pin)
            Pin(item);
    }

    void OnRoomChange()
    {
        g_NewItems.clear();
        g_MenuOpen = false;
        g_MouseOverUI = false;
        g_ButtonRef = -1.;
        g_ButtonCreated = false;
        g_Checkboxes.clear();
        g_MenuContentY = -1.;
    }

    bool IsButton(CInstance* inst)
    {
        return g_ButtonRef >= 0 && inst && inst->i_id == (int)g_ButtonRef;
    }

    void CreateButton()
    {
        g_ButtonCreated = true;
        g_ButtonShown = false;
        g_CreatingButton = true;
        g_ButtonRef = Binds::CallBuiltinA("instance_create_depth", { HiddenPos, HiddenPos, -15000., (double)LHObjectEnum::o_text_button });
        g_CreatingButton = false;
        Binds::SetVariable(g_ButtonRef, "click_event", -1.); // o_base_button runs event_perform(ev_other, click_event) on press
        Binds::SetVariable(g_ButtonRef, "sprite_index", (double)LHSpriteEnum::s_expsquarebutton);
        Binds::SetVariable(g_ButtonRef, "visible", false);
    }

    // Hidden while paused. Pause is global.menu_open, o_menu exists during the whole expedition and only slides in when paused.
    bool ShouldShowButton()
    {
        return !(bool)Binds::CallBuiltinA("variable_global_get", { "menu_open" });
    }

    void UpdateButton(bool shown)
    {
        double x = HiddenPos, y = HiddenPos;
        if (shown)
        {
            x = g_FreeCellX;
            y = g_FreeCellY;
        }

        if (shown == g_ButtonShown && x == g_ButtonX && y == g_ButtonY)
            return;

        if (shown != g_ButtonShown)
        {
            Binds::SetVariable(g_ButtonRef, "visible", shown);
        }
        g_ButtonShown = shown;
        g_ButtonX = x;
        g_ButtonY = y;
        Binds::SetVariable(g_ButtonRef, "x", x);
        Binds::SetVariable(g_ButtonRef, "y", y);
        if (!shown)
            g_MenuOpen = false;
    }

    // o_text_button_Alarm_6 fetches its text via __texts_all(text_type, text_key), then runs Alarm_7 which
    // lays out `text` for drawing. Clear it (the lock icon is drawn instead) and redo the layout.
    void ApplyButtonLabel(CInstance* button)
    {
        Binds::SetVariable(double(button->i_id), "text", "");
        YYRValue result;
        CallBuiltin(result, "event_perform", button, button, { 2., 7. }); // ev_alarm, 7
    }

    // Post gml_Object_o_text_button_Create_0
    void OnTextButtonCreated(CInstance* inst)
    {
        if (g_CreatingButton)
            ApplyButtonLabel(inst);
    }

    // Post gml_Object_o_text_button_Alarm_6
    void OnTextButtonTextReset(CInstance* inst)
    {
        if (IsButton(inst))
            ApplyButtonLabel(inst);
    }

    // Needs a draw event, surfaces can't be drawn to from the plugin thread
    void CreateIconSprite()
    {
        double surface = Binds::CallBuiltinA("surface_create", { IconCropSize, IconCropSize });
        Binds::CallBuiltinA("surface_set_target", { surface });
        Binds::CallBuiltinA("draw_clear_alpha", { 0., 0. });
        Binds::CallBuiltinA("draw_sprite_part_ext", { (double)LHSpriteEnum::s_equipmenu, 0., IconSourceX, IconSourceY, IconCropSize, IconCropSize,
            0., 0., 1., 1., 16777215., 1. });
        Binds::CallBuiltinA("surface_reset_target", {});
        g_IconSprite = Binds::CallBuiltinA("sprite_create_from_surface", { surface, 0., 0., IconCropSize, IconCropSize, true, false, 0., 0. });
        Binds::CallBuiltinA("surface_free", { surface });
    }

    // Post gml_Object_o_text_button_Draw_0
    void OnTextButtonDrawn(CInstance* inst)
    {
        if (!IsButton(inst) || !g_ButtonShown)
            return;

        if (g_IconSprite < 0)
            CreateIconSprite();

        double iconX = g_ButtonX + 1. + std::floor((ButtonSize - IconSize) / 2) - 1.;
        double iconY = g_ButtonY + std::floor((ButtonSize - IconSize) / 2);
        Binds::CallBuiltinA("draw_sprite", { g_IconSprite, 0., iconX, iconY });
    }

    // Post gml_Object_o_base_button_Mouse_4
    void OnButtonPressed(CInstance* inst)
    {
        if (IsButton(inst) && g_ButtonShown)
            g_MenuOpen = !g_MenuOpen;
    }

    // Pre patch, keeps clicks on the button and menu from reaching the game objects below them
    bool ShouldBlockEvent(const char* codeName, CInstance* self)
    {
        return g_MouseOverUI && !IsButton(self) && strstr(codeName, "_Mouse_") != nullptr;
    }

    bool PointInRect(double px, double py, double x1, double y1, double x2, double y2)
    {
        return px >= x1 && px <= x2 && py >= y1 && py <= y2;
    }

    double MenuContentHeight()
    {
        return MenuPadding * 2 + RowHeight * g_VisibleTypes.size();
    }

    double CheckboxX()
    {
        return g_MenuX + 8.;
    }

    double CheckboxY(int i)
    {
        return g_MenuContentY + MenuPadding + RowHeight * i + std::floor((RowHeight - CheckboxSize) / 2);
    }

    int CheckboxIndex(CInstance* inst)
    {
        if (!inst)
            return -1;
        for (size_t i = 0; i < g_Checkboxes.size(); i++)
        {
            if ((int)g_Checkboxes[i] == inst->i_id)
                return (int)i;
        }
        return -1;
    }

    void CreateCheckboxes()
    {
        g_Checkboxes.clear();
        for (int i = 0; i < (int)g_VisibleTypes.size(); i++)
        {
            double box = Binds::CallBuiltinA("instance_create_depth", { CheckboxX(), CheckboxY(i), CheckboxDepth, (double)LHObjectEnum::o_opt_button });
            Binds::SetVariable(box, "sprite_index", (double)LHSpriteEnum::s_opt_shriftgalki);
            Binds::SetVariable(box, "checked", g_TypeEnabled[g_VisibleTypes[i]]);
            // Only the o_opt_button children set these in their Create (e.g. o_opt_gamepad_button_Create_0),
            // o_opt_button_Step_2 crashes on the unset xx
            Binds::SetVariable(box, "xx", 0.);
            Binds::SetVariable(box, "yorig", CheckboxY(i));
            Binds::SetVariable(box, "yy", CheckboxY(i));
            g_Checkboxes.push_back(box);
        }
    }

    void DestroyCheckboxes()
    {
        for (double box : g_Checkboxes)
            Binds::CallBuiltinA("instance_destroy", { box });
        g_Checkboxes.clear();
    }

    // Right aligned below the button, the header ornament of s_dead_menu_toplut sits above its origin
    void LayoutMenu()
    {
        double menuX = std::clamp(g_ButtonX + 1. + ButtonSize - MenuWidth, 0., RoomWidth - MenuWidth);
        double top = std::clamp(g_ButtonY + ButtonSize + 2., 0., RoomHeight - MenuHeaderHeight - MenuContentHeight() - MenuBottomHeight);
        double contentY = top + MenuHeaderHeight;
        if (menuX == g_MenuX && contentY == g_MenuContentY)
            return;

        g_MenuX = menuX;
        g_MenuContentY = contentY;
        for (size_t i = 0; i < g_Checkboxes.size(); i++)
        {
            Binds::SetVariable(g_Checkboxes[i], "x", CheckboxX());
            Binds::SetVariable(g_Checkboxes[i], "y", CheckboxY((int)i));
        }
    }

    void DrawMenuBackground()
    {
        RefreshTypeLabels();
        LHDraw::DrawStateWrapper drawState;

        double contentH = MenuContentHeight();
        Binds::CallBuiltinA("draw_set_alpha", { 1. });
        Binds::CallBuiltinA("draw_sprite", { (double)LHSpriteEnum::s_dead_menu_toplut, 0., g_MenuX, g_MenuContentY });
        Binds::CallBuiltinA("draw_sprite_ext", { (double)LHSpriteEnum::s_dead_menu_midlut, 0., g_MenuX, g_MenuContentY, 1., contentH / MenuMidHeight, 0., 16777215., 1. });
        Binds::CallBuiltinA("draw_sprite", { (double)LHSpriteEnum::s_dead_menu_bot, 0., g_MenuX, g_MenuContentY + contentH });

        double font = Binds::CallBuiltinA("variable_global_get", { "font_mid" });
        if (font >= 0)
            Binds::CallBuiltinA("draw_set_font", { font });
        Binds::CallBuiltinA("draw_set_valign", { 1. }); // fa_middle

        Binds::CallBuiltinA("draw_set_halign", { 1. }); // fa_center
        LHDraw::DrawTextWrapper(g_MenuX + MenuWidth / 2, g_MenuContentY - MenuPlateHeight / 2, MenuTitle, ColorTitle);

        Binds::CallBuiltinA("draw_set_halign", { 0. }); // fa_left
        double labelX = CheckboxX() + CheckboxSize + 4.;
        for (int i = 0; i < (int)g_VisibleTypes.size(); i++)
        {
            int type = g_VisibleTypes[i];
            LHDraw::DrawTextWrapper(labelX, CheckboxY(i) + CheckboxSize / 2, g_TypeLabels[type], g_TypeEnabled[type] ? ColorTextEnabled : ColorText);
        }
    }

    // Pre gml_Object_o_opt_button_Step_0. o_menu's razdel_check() deactivates o_opt_buttons of other options tabs.
    void OnCheckboxStepBegin(CInstance* inst)
    {
        int i = CheckboxIndex(inst);
        if (i < 0)
            return;
        double box = double(inst->i_id);
        Binds::SetVariable(box, "active", 1.);
        g_CheckboxWasPressed[i] = Binds::GetVariable(box, "pressed");
    }

    // Post gml_Object_o_opt_button_Step_0. A click is a release while hovered and pressed, Step_0 then runs
    // event_perform(ev_other, 10) and resets pressed, but o_opt_button has no Other_10 to hook.
    void OnCheckboxStepEnd(CInstance* inst)
    {
        int i = CheckboxIndex(inst);
        if (i < 0)
            return;
        double box = double(inst->i_id);
        if (g_CheckboxWasPressed[i] && !(bool)Binds::GetVariable(box, "pressed") && (bool)Binds::GetVariable(box, "hover"))
        {
            int type = g_VisibleTypes[i];
            SetTypeEnabled(type, !g_TypeEnabled[type]);
            Binds::SetVariable(box, "checked", g_TypeEnabled[type]);
        }
    }

    // Pre patch. o_opt_button_Step_2 only moves o_opt_buttons along with o_menu, the checkboxes are placed by LayoutMenu.
    bool ShouldSkipCheckboxEvent(const char* codeName, CInstance* self)
    {
        return strcmp(codeName, "gml_Object_o_opt_button_Step_2") == 0 && CheckboxIndex(self) >= 0;
    }

    // Pre gml_Object_o_opt_button_Draw_0, the panel goes right below the first checkbox
    void OnCheckboxDraw(CInstance* inst)
    {
        if (CheckboxIndex(inst) < 0)
            return;
        Binds::SetVariable(double(inst->i_id), "active", 1.);
        if (!g_MenuBackgroundDrawn)
        {
            g_MenuBackgroundDrawn = true;
            DrawMenuBackground();
        }
    }

    // Runs in the draw end event, after all checkboxes are drawn for this frame
    void UpdateUI()
    {
        g_MenuBackgroundDrawn = false;
        g_MouseOverUI = false;

        if (!(bool)Binds::CallBuiltinA("instance_exists", { (double)LHObjectEnum::o_hero }))
            return;

        if (!g_ButtonCreated || (g_ButtonRef >= 0 && !(bool)Binds::CallBuiltinA("instance_exists", { g_ButtonRef })))
            CreateButton();

        if (g_ButtonRef < 0)
            return;

        bool shown = ShouldShowButton();
        UpdateButton(shown);
        if (!shown || !g_MenuOpen)
        {
            DestroyCheckboxes();
            if (!shown)
                return;
        }

        double mouseX = Binds::CallBuiltinA("device_mouse_x", { 0. });
        double mouseY = Binds::CallBuiltinA("device_mouse_y", { 0. });
        g_MouseOverUI = PointInRect(mouseX, mouseY, g_ButtonX + 1., g_ButtonY, g_ButtonX + 1. + ButtonSize, g_ButtonY + ButtonSize);

        if (!g_MenuOpen)
            return;

        if (!g_Checkboxes.empty() && DropListSize() != g_DropListSize)
            DestroyCheckboxes();

        if (g_Checkboxes.empty() || !(bool)Binds::CallBuiltinA("instance_exists", { g_Checkboxes[0] }))
        {
            RefreshVisibleTypes();
            g_MenuContentY = -1.; // row count may have changed, force LayoutMenu
            LayoutMenu();
            CreateCheckboxes();
        }
        LayoutMenu();

        double top = g_MenuContentY - MenuHeaderHeight;
        double bottom = g_MenuContentY + MenuContentHeight() + MenuBottomHeight;
        g_MouseOverUI = g_MouseOverUI || PointInRect(mouseX, mouseY, g_MenuX, top, g_MenuX + MenuWidth, bottom);
    }

    // Pre gml_Object_o_cursor_Draw_73
    void OnCursorDrawEnd()
    {
        UpdateUI();
    }

    // Post gml_Object_o_controller_Draw_73, for when the game doesn't draw its own cursor
    void OnControllerDrawEnd()
    {
        if (!(bool)Binds::CallBuiltinA("instance_exists", { (double)LHObjectEnum::o_cursor }))
            UpdateUI();
    }
}

