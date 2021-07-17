// world/shop.h -- The Shop class handles everything to do with shops that where the player can buy and sell items.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once

#include <cstddef>

#include <memory>
#include <string>

#include "world/inventory.h"


class Shop
{
public:
    static const std::string    SQL_SHOPS;  // SQL table construction string.

            Shop(uint32_t room_id);                                 // Constructor, sets up a blank shop by default.
    void    add_item(std::shared_ptr<Item> item, bool sort = true); // Adds an item to this shop's inventory.
    void    browse() const;                                         // Browses the wares on sale.
    void    buy(uint32_t id, int quantity);                         // Attempts to purchase something.
    const std::shared_ptr<Inventory>    inv() const;                // Returns a pointer to the shop's inventory.
    void    load(std::shared_ptr<SQLite::Database> save_db);        // Loads a shop from the save file.
    void    restock();                                              // Restocks the contents of this shop.
    void    save(std::shared_ptr<SQLite::Database> save_db) const;  // Saves this shop to the save file.
    void    sell(uint32_t id, int quantity, bool confirm);          // Offers an item to the shop to sell.

private:
    std::shared_ptr<Inventory>  m_inventory;    // The contents of this shop.
    uint32_t                    m_room_id;      // The room ID where this shop is located.
};
