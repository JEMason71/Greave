// actions/inventory.cpp -- Actions related to inventory management, picking up and dropping items, etc.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/inventory.hpp"
#include "core/core.hpp"
#include "core/guru.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/mobile.hpp"
#include "world/room.hpp"
#include "world/world.hpp"


const float ActionInventory::TIME_DROP_ITEM =       1.0f;   // The time taken (in seconds) to drop an item on the ground.
const float ActionInventory::TIME_EQUIP_ABOUT =     20.0f;  // The time taken (in seconds) to equip something about the body, like a cloak.
const float ActionInventory::TIME_EQUIP_ARMOUR =    180.0f; // The time taken (in seconds) to equip armour worn over the body, like a breastplate.
const float ActionInventory::TIME_EQUIP_BODY =      300.0f; // The time taken (in seconds) to equip armour worn against the body, like a hauberk.
const float ActionInventory::TIME_EQUIP_FEET =      30.0f;  // The time taken (in seconds) to equip boots or other things worn on the feet.
const float ActionInventory::TIME_EQUIP_HANDS =     6.0f;   // The time taken (in seconds) to equip gloves or something else worn on the hands.
const float ActionInventory::TIME_EQUIP_HEAD =      2.0f;   // The time taken (in seconds) to equip a helmet or so mething else worn on the head.
const float ActionInventory::TIME_EQUIP_WEAPON =    0.6f;   // The time taken (in seconds) to equip a weapon or something else held in the hand.
const float ActionInventory::TIME_GET_ITEM =        5.0f;   // The time taken (in seconds) to pick up an item from the ground.
const float ActionInventory::TIME_UNEQUIP_ABOUT =   10.0f;  // The time taken (in seconds) to unequip something about the body, like a cloak.
const float ActionInventory::TIME_UNEQUIP_ARMOUR =  120.0f; // The time taken (in seconds) to unequip armour worn over the body, like a breastplate.
const float ActionInventory::TIME_UNEQUIP_BODY =    180.0f; // The time taken (in seconds) to unequip armour worn against the body, like a hauberk.
const float ActionInventory::TIME_UNEQUIP_FEET =    15.0f;  // The time taken (in seconds) to unequip boots or other things worn on the feet.
const float ActionInventory::TIME_UNEQUIP_HANDS =   4.0f;   // The time taken (in seconds) to unequip gloves or something else worn on the hands.
const float ActionInventory::TIME_UNEQUIP_HEAD =    2.0f;   // The time taken (in seconds) to unequip a helmet or so mething else worn on the head.
const float ActionInventory::TIME_UNEQUIP_WEAPON =  0.5f;   // The time taken (in seconds) to unequip a weapon or something else held in the hand.


// Checks to see what's being carried.
void ActionInventory::check_inventory(std::shared_ptr<Mobile> mob)
{
    const auto inventory = mob->inv();
    const uint32_t inv_size = inventory->count();

    if (!inv_size)
    {
        core()->message("{y}You are not carrying anything.");
        return;
    }

    core()->message("{G}You are carrying:");
    for (unsigned int i = 0; i < inv_size; i++)
        core()->message("{0}" + inventory->get(i)->name(Item::ItemName::INVENTORY));
}

// Drops an item on the ground.
void ActionInventory::drop(std::shared_ptr<Mobile> mob, uint32_t item_pos)
{
    const std::shared_ptr<Item> item = mob->inv()->get(item_pos);
    const std::shared_ptr<Room> room = core()->world()->get_room(mob->location());

    if (!mob->pass_time(TIME_DROP_ITEM))
    {
        core()->message("{R}You are interrupted while attempting to drop " + item->name() + "{R}!");
        return;
    }

    mob->inv()->erase(item_pos);
    room->inv()->add_item(item);
    if (mob->is_player()) core()->message("{u}You drop " + item->name() + " {u}on the ground.");
    // todo: add message for NPCs dropping items
}

// Wields or wears an equippable item.
bool ActionInventory::equip(std::shared_ptr<Mobile> mob, uint32_t item_pos)
{
    const auto inv = mob->inv();
    const auto equ = mob->equ();
    const auto item = inv->get(item_pos);
    EquipSlot slot = item->equip_slot();
    const bool main_used = (equ->get(EquipSlot::HAND_MAIN) != nullptr);
    const bool off_used = (equ->get(EquipSlot::HAND_OFF) != nullptr);
    const bool two_handed_equipped = main_used && equ->get(EquipSlot::HAND_MAIN)->tag(ItemTag::TwoHanded);
    const bool two_handed_item = item->tag(ItemTag::TwoHanded);
    const bool prefer_off_hand = item->tag(ItemTag::PreferOffHand);
    const bool off_hand_only = item->tag(ItemTag::OffHandOnly);

    // Determine which slot to use for held items, since they may go in either hand.
    if (slot == EquipSlot::HAND_MAIN || slot == EquipSlot::HAND_OFF)
    {
        // Two-handed items require both hands to be empty.
        if (two_handed_item)
        {
            // If anything fails on unequipping, just bow out here.
            if ((main_used && !unequip(mob, EquipSlot::HAND_MAIN)) || (off_used && !unequip(mob, EquipSlot::HAND_OFF))) return false;
            slot = EquipSlot::HAND_MAIN;
        }
        // Normal one-handed equipping.
        else if (!prefer_off_hand && !off_hand_only)
        {
            // First, check if the main hand is free.
            if (!main_used) slot = EquipSlot::HAND_MAIN;

            // Failing that, check the off-hand is free *and* that the main-hand isn't two-handed.
            else if (!two_handed_equipped && !off_used) slot = EquipSlot::HAND_OFF;

            // Okay, both hands are used. First, try to unequip the main hand.
            else if (unequip(mob, EquipSlot::HAND_MAIN)) slot = EquipSlot::HAND_MAIN;

            // No luck? Okay, try to unequip the off-hand, if we're not using a two-hander.
            else if (!two_handed_equipped && unequip(mob, EquipSlot::HAND_OFF)) slot = EquipSlot::HAND_OFF;

            // Just give up if we can't even do that.
            else return false;
        }
        // Prefer off-hand is the same rules as above, just flipped.
        else
        {
            if (!two_handed_equipped && !off_used) slot = EquipSlot::HAND_OFF;
            else if (!main_used && !off_hand_only) slot = EquipSlot::HAND_MAIN;
            else if (!two_handed_equipped && unequip(mob, EquipSlot::HAND_OFF)) slot = EquipSlot::HAND_OFF;
            else if (!off_hand_only && unequip(mob, EquipSlot::HAND_MAIN)) slot = EquipSlot::HAND_MAIN;
            else return false;
        }

        // Update the Item's equipment slot.
        item->set_equip_slot(slot);
    }

    // Extra check for layered armour.
    if (slot == EquipSlot::BODY && equ->get(EquipSlot::ARMOUR))
    {
        if (mob->is_player())
        {
            core()->message("{y}You'll need to remove your {Y}" + equ->get(EquipSlot::ARMOUR)->name() + " {y}first.");
            return false;
        }
        // For NPCs, just unequip the outer armour first.
        if (!unequip(mob, EquipSlot::ARMOUR)) return false;
    }

    // Attempt to unequip the item currently using the slot. If it's not possible (cursed?), just stop here.
    if (equ->get(slot) != nullptr && !unequip(mob, slot)) return false;

    std::string action = "wear", slot_name;
    float time_taken = 0;
    switch (slot)
    {
        case EquipSlot::NONE: case EquipSlot::_END: throw std::runtime_error("Attempt to equip item into null slot.");
        case EquipSlot::ABOUT_BODY:
            slot_name = "about %your% body";
            time_taken = TIME_EQUIP_ABOUT;
            break;
        case EquipSlot::ARMOUR:
            slot_name = "over %your% body";
            time_taken = TIME_EQUIP_ARMOUR;
            break;
        case EquipSlot::BODY:
            slot_name = "on %your% body";
            time_taken = TIME_EQUIP_BODY;
            break;
        case EquipSlot::FEET:
            slot_name = "on %your% feet";
            time_taken = TIME_EQUIP_FEET;
            break;
        case EquipSlot::HAND_MAIN:
            if (two_handed_item || (item->tag(ItemTag::HandAndAHalf) && !mob->equ()->get(EquipSlot::HAND_OFF))) slot_name = "in both hands";
            else slot_name = "in %your% main hand";
            action = "hold";
            time_taken = TIME_EQUIP_WEAPON;
            break;
        case EquipSlot::HAND_OFF:
            slot_name = "in %your% off-hand";
            action = "hold";
            time_taken = TIME_EQUIP_WEAPON;
            break;
        case EquipSlot::HANDS:
            slot_name = "on %your% hands";
            time_taken = TIME_EQUIP_HANDS;
            break;
        case EquipSlot::HEAD:
            slot_name = "on %your% head";
            time_taken = TIME_EQUIP_HEAD;
            break;
    }
    if (!time_taken) core()->guru()->nonfatal("Could not determine equip time for " + item->name(), Guru::ERROR);
    // todo: messages for NPCs equipping gear
    StrX::find_and_replace(slot_name, "%your%", "your");

    const bool success = mob->pass_time(time_taken);
    if (!success)
    {
        core()->message("{R}You are interrupted while attempting to " + action + " the " + item->name() + "{R}!", Show::ALWAYS, Wake::ALWAYS);
        return false;
    }
    core()->message("{U}You " + action + " the " + item->name() + " {U}" + slot_name + ".");

    const auto room = core()->world()->get_room(mob->location());
    equ->add_item(item);
    inv->remove_item(item_pos);

    return true;
}

// Checks to see what's wielded and/or worn.
void ActionInventory::equipment(std::shared_ptr<Mobile> mob)
{
    const auto equ = mob->equ();
    if (!equ->count())
    {
        core()->message("{y}You aren't {Y}wearing or wielding {y}anything.");
        return;
    }

    core()->message("{G}Your equipment:");
    for (unsigned int i = 1; i < static_cast<unsigned int>(EquipSlot::_END); i++)
    {
        const EquipSlot es = static_cast<EquipSlot>(i);
        const auto item = equ->get(es);
        if (!item) continue;
        std::string slot_name;
        switch (es)
        {
            case EquipSlot::NONE: case EquipSlot::_END: break;
            case EquipSlot::ABOUT_BODY: slot_name = "about body"; break;
            case EquipSlot::ARMOUR: slot_name = "over body"; break;
            case EquipSlot::BODY: slot_name = "on body"; break;
            case EquipSlot::FEET: slot_name = "on feet"; break;
            case EquipSlot::HAND_MAIN:
                if (item->tag(ItemTag::TwoHanded) || (item->tag(ItemTag::HandAndAHalf) && !equ->get(EquipSlot::HAND_OFF))) slot_name = "in both hands";
                else slot_name = "in main hand";
                break;
            case EquipSlot::HAND_OFF:
                if (item->tag(ItemTag::HandAndAHalf) && !equ->get(EquipSlot::HAND_MAIN)) slot_name = "in both hands";
                else slot_name = "in off-hand";
                break;
            case EquipSlot::HANDS: slot_name = "on hands"; break;
            case EquipSlot::HEAD: slot_name = "on head"; break;
        }
        core()->message("{0}" + item->name(Item::ItemName::INVENTORY) + " {B}(" + slot_name + ")");
    }
}

// Takes an item from the ground.
void ActionInventory::take(std::shared_ptr<Mobile> mob, uint32_t item_pos)
{
    const std::shared_ptr<Room> room = core()->world()->get_room(mob->location());
    const std::shared_ptr<Item> item = room->inv()->get(item_pos);

    if (!mob->pass_time(TIME_GET_ITEM))
    {
        core()->message("{R}You are interrupted while trying to pick up " + item->name() + "{R}!");
        return;
    }

    room->inv()->erase(item_pos);
    mob->inv()->add_item(item);
    if (mob->is_player()) core()->message("{u}You pick up " + item->name() + "{u}.");
    // todo: add message for NPCs taking items
}

// Unequips a worn or wielded item.
bool ActionInventory::unequip(std::shared_ptr<Mobile> mob, uint32_t item_pos)
{
    const auto equ = mob->equ();
    const auto inv = mob->inv();
    if (item_pos >= equ->count()) throw std::runtime_error("Invalid equipment vector position.");
    const auto item = equ->get(item_pos);
    const EquipSlot slot = item->equip_slot();

    if (slot == EquipSlot::BODY && equ->get(EquipSlot::ARMOUR))
    {
        if (mob->is_player())
        {
            core()->message("{y}You'll need to remove your {Y}" + equ->get(EquipSlot::ARMOUR)->name() + " {y}first.");
            return false;
        }
        // For NPCs, just unequip the outer armour first.
        if (!unequip(mob, EquipSlot::ARMOUR)) return false;
    }

    // todo: add message for NPCs unequipping items
    std::string action = "remove";
    if (slot == EquipSlot::HAND_MAIN || slot == EquipSlot::HAND_OFF) action = "stop holding";
    float time_taken = 0;
    switch (slot)
    {
        case EquipSlot::NONE: case EquipSlot::_END: throw std::runtime_error("Invalid equipment slot.");
        case EquipSlot::ABOUT_BODY: time_taken = TIME_UNEQUIP_ABOUT; break;
        case EquipSlot::ARMOUR: time_taken = TIME_UNEQUIP_ARMOUR; break;
        case EquipSlot::BODY: time_taken = TIME_UNEQUIP_BODY; break;
        case EquipSlot::FEET: time_taken = TIME_UNEQUIP_FEET; break;
        case EquipSlot::HAND_MAIN: case EquipSlot::HAND_OFF: time_taken = TIME_UNEQUIP_WEAPON; break;
        case EquipSlot::HANDS: time_taken = TIME_UNEQUIP_HANDS; break;
        case EquipSlot::HEAD: time_taken = TIME_UNEQUIP_HEAD; break;
    }
    if (!time_taken) core()->guru()->nonfatal("Could not determine unequip time for " + item->name(), Guru::ERROR);
    const bool success = mob->pass_time(time_taken);
    if (!success)
    {
        core()->message("{R}You are interrupted while attempting to " + action + " the " + item->name() + "{R}!", Show::ALWAYS, Wake::ALWAYS);
        return false;
    }

    core()->message("{U}You " + action + " your " + item->name() + "{U}.");
    equ->remove_item(item_pos);
    inv->add_item(item);
    return true;
}

// As above, but specifying an EquipSlot.
bool ActionInventory::unequip(std::shared_ptr<Mobile> mob, EquipSlot slot)
{
    const auto inv = mob->equ();
    for (unsigned int i = 0; i < inv->count(); i++)
        if (inv->get(i)->equip_slot() == slot) return unequip(mob, i);
    return false;
}
