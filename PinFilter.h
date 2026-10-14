#pragma once
#include "PluginSetup.h"

// Auto-pins fresh drops of the item types selected in a menu opened from a button in the equipment grid
namespace PinFilter {
    void LoadConfig();

    // Pre patch, returning true cancels the original event
    bool ShouldBlockEvent(const char* codeName, CInstance* self);
    bool ShouldSkipCheckboxEvent(const char* codeName, CInstance* self);

    void OnRoomChange();
    void OnItemGenerated(CInstance* item);
    void OnItemPlaced(CInstance* itemInst);
    void OnButtonPressed(CInstance* inst);
    void OnTextButtonCreated(CInstance* inst);
    void OnTextButtonTextReset(CInstance* inst);
    void OnTextButtonDrawn(CInstance* inst);
    void OnCheckboxStepBegin(CInstance* inst);
    void OnCheckboxStepEnd(CInstance* inst);
    void OnCheckboxDraw(CInstance* inst);
    void OnCursorDrawEnd();
    void OnControllerDrawEnd();
}
