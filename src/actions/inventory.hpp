// actions/inventory.hpp -- Actions related to inventory management, picking up and dropping items, etc.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


class Item;                     // defined in world/item.hpp
class Mobile;                   // defined in world/mobile.hpp
enum class EquipSlot : uint8_t; // defined in world/item.hpp


class ActionInventory
{
public:
    static void check_inventory();                                                              // Checks to see what's being carried.
    static void drop(std::shared_ptr<Mobile> mob, size_t item_pos, int count, bool confirm);    // Drops an item on the ground.
    static bool equip(std::shared_ptr<Mobile> mob, size_t item_pos, bool confirm);              // Wields or wears an equippable item.
    static void equipment();                                                                    // Checks to see what's wielded and/or worn.
    static void take(std::shared_ptr<Mobile> mob, size_t item_pos, int count, bool confirm);    // Takes an item from the ground.
    static bool unequip(std::shared_ptr<Mobile> mob, size_t item_pos, bool confirm);            // Unequips a worn or wielded item.
    static bool unequip(std::shared_ptr<Mobile> mob, EquipSlot slot, bool confirm);             // As above, but specifying an EquipSlot.

private:
    static const float  TIME_DROP_ITEM;         // The time taken (in seconds) to drop an item on the ground.
    static const float  TIME_EQUIP_ABOUT;       // The time taken (in seconds) to equip something about the body, like a cloak.
    static const float  TIME_EQUIP_ARMOUR;      // The time taken (in seconds) to equip armour worn over the body, like a breastplate.
    static const float  TIME_EQUIP_BODY;        // The time taken (in seconds) to equip armour worn against the body, like a hauberk.
    static const float  TIME_EQUIP_FEET;        // The time taken (in seconds) to equip boots or other things worn on the feet.
    static const float  TIME_EQUIP_HANDS;       // The time taken (in seconds) to equip gloves or something else worn on the hands.
    static const float  TIME_EQUIP_HEAD;        // The time taken (in seconds) to equip a helmet or so mething else worn on the head.
    static const float  TIME_EQUIP_WEAPON;      // The time taken (in seconds) to equip a weapon or something else held in the hand.
    static const float  TIME_GET_ITEM;          // The time taken (in seconds) to pick up an item from the ground.
    static const float  TIME_UNEQUIP_ABOUT;     // The time taken (in seconds) to unequip something about the body, like a cloak.
    static const float  TIME_UNEQUIP_ARMOUR;    // The time taken (in seconds) to unequip armour worn over the body, like a breastplate.
    static const float  TIME_UNEQUIP_BODY;      // The time taken (in seconds) to unequip armour worn against the body, like a hauberk.
    static const float  TIME_UNEQUIP_FEET;      // The time taken (in seconds) to unequip boots or other things worn on the feet.
    static const float  TIME_UNEQUIP_HANDS;     // The time taken (in seconds) to unequip gloves or something else worn on the hands.
    static const float  TIME_UNEQUIP_HEAD;      // The time taken (in seconds) to unequip a helmet or so mething else worn on the head.
    static const float  TIME_UNEQUIP_WEAPON;    // The time taken (in seconds) to unequip a weapon or something else held in the hand.

    static bool count_check(std::shared_ptr<Item> item, int count); // Checks that a player-input count is a valid number.
    static void weight_and_money();                                 // Shows the total carry weight and currency the Mobile is carrying.
};
