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
