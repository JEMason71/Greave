// world/item.hpp -- The Item class is for objects that can be picked up and used by the player or other NPCs.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

namespace SQLite { class Database; }    // defined in 3rdparty/SQLiteCpp/Database.h


// ItemType is the primary type of Item (e.g. weapon, food, etc.)
enum class ItemType : uint16_t { NONE };

// ItemSub is for sub-types of items, e.g. a tool could sub-classify itself here.
enum class ItemSub : uint16_t { NONE };

class Item
{
public:
    static const std::string    SQL_ITEMS;  // The SQL table construction string for saving items.

                Item(); // Constructor, sets default values.
    static std::shared_ptr<Item> load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id);  // Loads a new Item from the save file.
    std::string name() const;   // Retrieves the name of thie Item.
    void        save(std::shared_ptr<SQLite::Database> save_db);    // Saves the Item to the save file.
    void        set_name(const std::string &name);  // Sets the name of this Item.
    void        set_type(ItemType type, ItemSub sub = ItemSub::NONE);   // Sets the type of this Item.
    ItemSub     subtype() const;    // Returns the ItemSub (sub-type) of this Item.
    ItemType    type() const;   // Returns the ItemType of this Item.

private:
    std::string m_name; // The name of this Item!
    ItemType    m_type; // The primary type of this Item.
    ItemSub     m_type_sub; // The subtype of this Item, if any.
};
