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


// Checks to see what's being carried.
void ActionInventory::check_inventory(std::shared_ptr<Mobile> mob)
{
    if (mob->type() != Mobile::Type::PLAYER)
    {
        core()->guru()->nonfatal("Attempt to check inventory on non-player Mobile.", Guru::WARN);
        return;
    }

    const auto inventory = mob->inv();
    const uint32_t inv_size = inventory->count();
    
    if (!inv_size)
    {
        core()->message("{y}You are not carrying anything.");
        return;
    }

    core()->message("{G}You are carrying:");
    for (unsigned int i = 0; i < inv_size; i++)
    {
        const std::shared_ptr<Item> item = inventory->get(i);
        std::string item_name = item->name();
        item_name += " {B}{" + StrX::itoh(item->hex_id(), 3) + "}";
        core()->message("{0}" + item_name);
    }
}

// Drops an item on the ground.
void ActionInventory::drop(std::shared_ptr<Mobile> mob, uint32_t item_pos)
{
    const std::shared_ptr<Item> item = mob->inv()->get(item_pos);
    const std::shared_ptr<Room> room = core()->world()->get_room(mob->location());
    mob->inv()->erase(item_pos);
    room->inv()->add_item(item);
    if (mob->type() == Mobile::Type::PLAYER) core()->message("{u}You drop " + item->name() + " {u}on the ground.");
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
        else
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

        // Update the Item's equipment slot.
        item->set_equip_slot(slot);
    }

    // Attempt to unequip the item currently using the slot. If it's not possible (cursed?), just stop here.
    if (equ->get(slot) != nullptr && !unequip(mob, slot)) return false;

    std::string action = "wear", slot_name;
    switch (slot)
    {
        case EquipSlot::NONE: case EquipSlot::_END: throw std::runtime_error("Attempt to equip item into null slot.");
        case EquipSlot::ABOUT_BODY: slot_name = "about %your% body"; break;
        case EquipSlot::ARMOUR: slot_name = "over %your% body"; break;
        case EquipSlot::BODY: slot_name = "on %your% body"; break;
        case EquipSlot::FEET: slot_name = "on %your% feet"; break;
        case EquipSlot::HAND_MAIN:
            if (two_handed_item) slot_name = "in both hands";
            else slot_name = "in %your% main hand";
            action = "hold";
            break;
        case EquipSlot::HAND_OFF: slot_name = "in %your% off-hand"; action = "hold"; break;
        case EquipSlot::HANDS: slot_name = "on %your% hands"; break;
        case EquipSlot::HEAD: slot_name = "on %your% head"; break;
    }
    // todo: messages for NPCs equipping gear
    StrX::find_and_replace(slot_name, "%your%", "your");
    core()->message("{U}You " + action + " the " + item->name() + " {U}" + slot_name + ".");
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
                if (item->tag(ItemTag::TwoHanded)) slot_name = "in both hands";
                else slot_name = "in main hand";
                break;
            case EquipSlot::HAND_OFF: slot_name = "in off-hand"; break;
            case EquipSlot::HANDS: slot_name = "on hands"; break;
            case EquipSlot::HEAD: slot_name = "on head"; break;
        }
        core()->message("{0}" + item->name() + " {B}(" + slot_name + ")");
    }
}

// Takes an item from the ground.
void ActionInventory::take(std::shared_ptr<Mobile> mob, uint32_t item_pos)
{
    const std::shared_ptr<Room> room = core()->world()->get_room(mob->location());
    const std::shared_ptr<Item> item = room->inv()->get(item_pos);
    room->inv()->erase(item_pos);
    mob->inv()->add_item(item);
    if (mob->type() == Mobile::Type::PLAYER) core()->message("{u}You pick up " + item->name() + "{u}.");
    // todo: add message for NPCs taking items
}

// Unequips a worn or wielded item.
bool ActionInventory::unequip(std::shared_ptr<Mobile> mob, uint32_t item_pos)
{
    const auto equ = mob->equ();
    const auto inv = mob->inv();
    if (item_pos >= equ->count()) throw std::runtime_error("Invalid equipment vector position.");
    const auto item = equ->get(item_pos);

    // todo: add message for NPCs unequipping items
    std::string action = "remove";
    if (item->equip_slot() == EquipSlot::HAND_MAIN || item->equip_slot() == EquipSlot::HAND_OFF) action = "stop holding";
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
