// world/inventory.hpp -- The Inventory class stores a collection of Items, and handles stacking, organizing, saving/loading, etc.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Item; // defined in world/item.hpp
namespace SQLite { class Database; }    // defined in 3rdparty/SQLiteCpp/Database.h


class Inventory
{
public:
    unsigned int    count() const;  // Returns the number of Items in this Inventory.
    std::shared_ptr<Item> get(uint32_t pos) const;  // Retrieves an Item from this Inventory.
    void            load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id);   // Loads an Inventory from the save file.
    uint32_t        save(std::shared_ptr<SQLite::Database> save_db);    // Saves this Inventory, returns its SQL ID.

private:
    std::vector<std::shared_ptr<Item>>  m_items;    // The Items stored in this Inventory.
};
