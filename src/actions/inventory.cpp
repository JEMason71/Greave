// actions/inventory.cpp -- Actions related to inventory management, picking up and dropping items, etc.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/inventory.hpp"
#include "core/core.hpp"
#include "core/guru.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/mobile.hpp"


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
