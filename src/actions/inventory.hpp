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

private:
    static const float  EQUIP_TIME_ABOUT;       // The time taken (in seconds) to equip something about the body, like a cloak.
    static const float  EQUIP_TIME_ARMOUR;      // The time taken (in seconds) to equip armour worn over the body, like a breastplate.
    static const float  EQUIP_TIME_BODY;        // The time taken (in seconds) to equip armour worn against the body, like a hauberk.
    static const float  EQUIP_TIME_FEET;        // The time taken (in seconds) to equip boots or other things worn on the feet.
    static const float  EQUIP_TIME_HANDS;       // The time taken (in seconds) to equip gloves or something else worn on the hands.
    static const float  EQUIP_TIME_HEAD;        // The time taken (in seconds) to equip a helmet or so mething else worn on the head.
    static const float  EQUIP_TIME_WEAPON;      // The time taken (in seconds) to equip a weapon or something else held in the hand.
    static const float  UNEQUIP_TIME_ABOUT;     // The time taken (in seconds) to unequip something about the body, like a cloak.
    static const float  UNEQUIP_TIME_ARMOUR;    // The time taken (in seconds) to unequip armour worn over the body, like a breastplate.
    static const float  UNEQUIP_TIME_BODY;      // The time taken (in seconds) to unequip armour worn against the body, like a hauberk.
    static const float  UNEQUIP_TIME_FEET;      // The time taken (in seconds) to unequip boots or other things worn on the feet.
    static const float  UNEQUIP_TIME_HANDS;     // The time taken (in seconds) to unequip gloves or something else worn on the hands.
    static const float  UNEQUIP_TIME_HEAD;      // The time taken (in seconds) to unequip a helmet or so mething else worn on the head.
    static const float  UNEQUIP_TIME_WEAPON;    // The time taken (in seconds) to unequip a weapon or something else held in the hand.
};
