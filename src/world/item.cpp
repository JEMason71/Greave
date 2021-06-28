// world/item.cpp -- The Item class is for objects that can be picked up and used by the player or other NPCs.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.hpp"
#include "world/item.hpp"
#include "world/world.hpp"


// The SQL table construction string for saving items.
const std::string Item::SQL_ITEMS = "CREATE TABLE items ( sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL, name TEXT NOT NULL, type INTEGER, subtype INTEGER )";


// Constructor, sets default values.
Item::Item() : m_type(ItemType::NONE), m_type_sub(ItemSub::NONE) { }

// Loads a new Item from the save file.
std::shared_ptr<Item> Item::load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id)
{
    auto new_item = std::make_shared<Item>();

    SQLite::Statement query(*save_db, "SELECT FROM items WHERE sql_id = ?");
    query.bind(1, sql_id);
    if (query.executeStep())
    {
        new_item->set_name(query.getColumn("name").getString());
        ItemType new_type = ItemType::NONE;
        ItemSub new_subtype = ItemSub::NONE;
        if (!query.isColumnNull("type")) new_type = static_cast<ItemType>(query.getColumn("type").getInt());
        if (!query.isColumnNull("subtype")) new_subtype = static_cast<ItemSub>(query.getColumn("subtype").getInt());
        new_item->set_type(new_type, new_subtype);
    }
    else throw std::runtime_error("Could not retrieve data for item ID " + std::to_string(sql_id));

    return new_item;
}

// Retrieves the name of thie Item.
std::string Item::name() const { return m_name; }

// Saves the Item.
void Item::save(std::shared_ptr<SQLite::Database> save_db)
{
    SQLite::Statement query(*save_db, "INSERT INTO items ( sql_id, name, type, subtype ) VALUES ( ?, ?, ?, ? )");
    query.bind(1, core()->sql_unique_id());
    query.bind(2, m_name);
    query.bind(3, static_cast<int>(m_type));
    query.bind(4, static_cast<int>(m_type_sub));
    query.exec();
}

// Sets the name of this Item.
void Item::set_name(const std::string &name) { m_name = name; }

// Sets the type of this Item.
void Item::set_type(ItemType type, ItemSub sub)
{
    m_type = type;
    m_type_sub = sub;
}

// Returns the ItemSub (sub-type) of this Item.
ItemSub Item::subtype() const { return m_type_sub; }

// Returns the ItemType of this Item.
ItemType Item::type() const { return m_type; }
