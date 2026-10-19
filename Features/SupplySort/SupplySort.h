#pragma once
#include "../../PluginSetup.h"

// Makes the sort button of the camp supply menu also sort the equipped supplies, not only the stored ones
namespace SupplySort {
    void OnSortPressed();
    void OnSlotStep(CInstance* slot);
}
