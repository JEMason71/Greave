// world/inventory.cpp -- The Inventory class stores a collection of Items, and handles stacking, organizing, saving/loading, etc.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"


// Returns the number of Items in this Inventory.
unsigned int Inventory::count() const { return m_items.size(); }

// Retrieves an Item from this Inventory.
std::shared_ptr<Item> Inventory::get(uint32_t pos) const
{
    if (pos >= m_items.size()) throw std::runtime_error("Invalid inventory position requested.");
    return m_items.at(pos);
}

// Loads an Inventory from the save file.
void Inventory::load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id)
{
    m_items.clear();
    SQLite::Statement query(*save_db, "SELECT sql_id FROM items WHERE owner_id = ? ORDER BY sql_id ASC");
    query.bind(1, sql_id);
    bool loaded_items = false;
    while (query.executeStep())
    {
        auto new_item = Item::load(save_db, query.getColumn("sql_id").getUInt());
        m_items.push_back(new_item);
    }
    if (!loaded_items) throw std::runtime_error("Could not load inventory data " + std::to_string(sql_id));
}

// Saves this Inventory, returns its SQL ID.
uint32_t Inventory::save(std::shared_ptr<SQLite::Database> save_db)
{
    if (!m_items.size()) return 0;
    const uint32_t sql_id = core()->sql_unique_id();
    for (unsigned int i = 0; i < m_items.size(); i++)
        m_items.at(i)->save(save_db, sql_id);
    return sql_id;
}
