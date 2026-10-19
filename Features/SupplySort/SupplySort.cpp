#include "SupplySort.h"
#include <vector>
#include <unordered_set>
#include <algorithm>
#include "../../LH/LHObjects.h"

#define SLOT_EMPTY -1.

namespace SupplySort {

    // list of display slots that need updating
    static std::unordered_set<int> g_StaleSlots;

    // Sort filled slots and fill up free slots with SLOT_EMPTY
    void SortEquipList(double list)
    {
        if (!(bool)Binds::CallBuiltinA("ds_exists", { list, 2. })) // ds_type_list
            return;

        int size = (int)(double)Binds::CallBuiltinA("ds_list_size", { list });
        std::vector<double> items;
        for (int i = 0; i < size; i++)
        {
            // iterate the dslist
            double item = Binds::CallBuiltinA("ds_list_find_value", { list, (double)i });
            if (item > SLOT_EMPTY) // not empty
            {
                items.push_back(item);
            }
        }
        std::sort(items.begin(), items.end());

        for (int i = 0; i < size; i++)
        {
            // set values to the sorted list and fill up with -1s
            double item = i < (int)items.size() ? items[i] : SLOT_EMPTY;
            Binds::CallBuiltinA("ds_list_set", { list, (double)i, item });
        }
    }

    // Post gml_Object_o_camp_equipmenu_sort_Other_10
    void OnSortPressed()
    {
        // One list per supply type at index 1-4, index 0 is unused
        YYRValue lists = Binds::CallBuiltinA("variable_global_get", { "_equip_lists" });

        if (!(bool)Binds::CallBuiltinA("is_array", { lists }))
            return;

        int count = (int)(double)Binds::CallBuiltinA("array_length", { lists });

        for (int type = 1; type < count; type++)
        {
            // sort each list starting from index 1
            SortEquipList(Binds::CallBuiltinA("array_get", { lists, (double)type }));
        }

        // reset slots to redraw
        g_StaleSlots.clear();
        double slotObj = (double)LHObjectEnum::o_camp_equip_slots;
        int slots = (int)(double)Binds::CallBuiltinA("instance_number", { slotObj });
        for (int i = 0; i < slots; i++) // add all of o_camp_equip_slots
        {
            g_StaleSlots.insert((int)(double)Binds::CallBuiltinA("instance_find", { slotObj, (double)i }));
        }   
    }

    // Post gml_Object_o_camp_equip_slots_Step_0, event_perform needs the slot instance itself
    void OnSlotStep(CInstance* slot)
    {
        if (g_StaleSlots.erase(slot->i_id) == 0)
            return;
        YYRValue result;
        CallBuiltin(result, "event_perform", slot, slot, { 2., 0. }); // ev_alarm, 0
    }
}
