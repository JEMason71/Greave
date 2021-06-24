// world/mobile.cpp -- The Mobile class defines entities that can move and interact with the game world. Derived classes are used for more specific entities.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/strx.hpp"
#include "world/mobile.hpp"


// The SQL table construction string for Mobiles.
const std::string   Mobile::SQL_MOBILES =   "CREATE TABLE mobiles ( sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL, location INTEGER NOT NULL )";


// Constructor, sets default values.
Mobile::Mobile() : m_location(0) { }

// Loads a Mobile.
void Mobile::load(std::shared_ptr<SQLite::Database> save_db, unsigned int sql_id)
{
    SQLite::Statement query(*save_db, "SELECT * FROM mobiles WHERE sql_id = ?");
    query.bind(1, sql_id);
    if (query.executeStep())
    {
        m_location = query.getColumn("location").getUInt();
    }
    else throw std::runtime_error("Could not load mobile data!");
}

// Retrieves the location of this Mobile, in the form of a Room ID.
uint32_t Mobile::location() const { return m_location; }

// Saves this Mobile.
uint32_t Mobile::save(std::shared_ptr<SQLite::Database> save_db)
{
    const uint32_t sql_id = core()->sql_unique_id();
    SQLite::Statement query(*save_db, "INSERT INTO mobiles ( sql_id, location ) VALUES ( ?, ? )");
    query.bind(1, sql_id);
    query.bind(2, m_location);
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
