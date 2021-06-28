// actions/inventory.hpp -- Actions related to inventory management, picking up and dropping items, etc.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Mobile;   // defined in world/mobile.hpp


class ActionInventory
{
public:
    static void check_inventory(std::shared_ptr<Mobile> mob);           // Checks to see what's being carried.
    static void drop(std::shared_ptr<Mobile> mob, uint32_t item_pos);   // Drops an item on the ground.
    static void take(std::shared_ptr<Mobile> mob, uint32_t item_pos);   // Takes an item from the ground.
};
