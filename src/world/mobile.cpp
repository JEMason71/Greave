// world/mobile.cpp -- The Mobile class defines entities that can move and interact with the game world. Derived classes are used for more specific entities.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/mobile.hpp"


// The SQL table construction string for Mobiles.
const std::string   Mobile::SQL_MOBILES =   "CREATE TABLE mobiles ( sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL, location INTEGER NOT NULL, inventory INTEGER UNIQUE )";


// Constructor, sets default values.
Mobile::Mobile() : m_inventory(std::make_shared<Inventory>()), m_location(0) { }

// Loads a Mobile.
void Mobile::load(std::shared_ptr<SQLite::Database> save_db, unsigned int sql_id)
{
    uint32_t inventory_id = 0;
    SQLite::Statement query(*save_db, "SELECT * FROM mobiles WHERE sql_id = ?");
    query.bind(1, sql_id);
    if (query.executeStep())
    {
        m_location = query.getColumn("location").getUInt();
        if (!query.isColumnNull("inventory")) inventory_id = query.getColumn("inventory").getUInt();
    }
    else throw std::runtime_error("Could not load mobile data!");

    if (inventory_id) m_inventory->load(save_db, inventory_id);
}

// Retrieves the location of this Mobile, in the form of a Room ID.
uint32_t Mobile::location() const { return m_location; }

// Saves this Mobile.
uint32_t Mobile::save(std::shared_ptr<SQLite::Database> save_db)
{
    const uint32_t inventory_id = m_inventory->save(save_db);

    const uint32_t sql_id = core()->sql_unique_id();
    SQLite::Statement query(*save_db, "INSERT INTO mobiles ( sql_id, location, inventory ) VALUES ( ?, ?, ? )");
    query.bind(1, sql_id);
    query.bind(2, m_location);
    if (inventory_id) query.bind(3, inventory_id);
    query.exec();
    return sql_id;
}

// Sets the location of this Mobile with a Room ID.
void Mobile::set_location(uint32_t room_id) { m_location = room_id; }

// As above, but with a string Room ID.
void Mobile::set_location(const std::string &room_id)
{
    if (!room_id.size()) set_location(0);
    else set_location(StrX::hash(room_id));
}
