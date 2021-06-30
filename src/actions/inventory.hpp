// actions/inventory.hpp -- Actions related to inventory management, picking up and dropping items, etc.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


class Mobile;   // defined in world/mobile.hpp
enum class EquipSlot : uint8_t;         // defined in world/item.hpp


class ActionInventory
{
public:
    static void check_inventory(std::shared_ptr<Mobile> mob);               // Checks to see what's being carried.
    static void drop(std::shared_ptr<Mobile> mob, uint32_t item_pos);       // Drops an item on the ground.
    static bool equip(std::shared_ptr<Mobile> mob, uint32_t item_pos);      // Wields or wears an equippable item.
    static void equipment(std::shared_ptr<Mobile> mob);                     // Checks to see what's wielded and/or worn.
    static void take(std::shared_ptr<Mobile> mob, uint32_t item_pos);       // Takes an item from the ground.
    static bool unequip(std::shared_ptr<Mobile> mob, uint32_t item_pos);    // Unequips a worn or wielded item.
    static bool unequip(std::shared_ptr<Mobile> mob, EquipSlot slot);       // As above, but specifying an EquipSlot.
};
