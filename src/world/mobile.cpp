// world/mobile.cpp -- The Mobile class defines entities that can move and interact with the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/mobile.hpp"
#include "world/time-weather.hpp"
#include "world/world.hpp"


// The SQL table construction string for Mobiles.
const std::string   Mobile::SQL_MOBILES =   "CREATE TABLE mobiles ( action_timer REAL, equipment INTEGER UNIQUE, inventory INTEGER UNIQUE, location INTEGER NOT NULL, name TEXT, "
    "sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL )";


// Constructor, sets default values.
Mobile::Mobile() : m_action_timer(0), m_equipment(std::make_shared<Inventory>()), m_inventory(std::make_shared<Inventory>()), m_location(0) { }

// Returns a pointer to the Movile's equipment.
const std::shared_ptr<Inventory> Mobile::equ() const { return m_equipment; }

// Returns a pointer to the Mobile's Inventory.
const std::shared_ptr<Inventory> Mobile::inv() const { return m_inventory; }

// Returns true if this Mobile is a Player, false if not.
bool Mobile::is_player() const { return false; }

// Loads a Mobile.
uint32_t Mobile::load(std::shared_ptr<SQLite::Database> save_db, unsigned int sql_id)
{
    uint32_t inventory_id = 0, equipment_id = 0;
    SQLite::Statement query(*save_db, "SELECT * FROM mobiles WHERE sql_id = ?");
    query.bind(1, sql_id);
    if (query.executeStep())
    {
        if (!query.isColumnNull("action_timer")) m_action_timer = query.getColumn("action_timer").getDouble();
        if (!query.isColumnNull("equipment")) equipment_id = query.getColumn("equipment").getUInt();
        if (!query.isColumnNull("inventory")) inventory_id = query.getColumn("inventory").getUInt();
        m_location = query.getColumn("location").getUInt();
        if (!query.isColumnNull("name")) m_name = query.getColumn("name").getString();
    }
    else throw std::runtime_error("Could not load mobile data!");

    if (inventory_id) m_inventory->load(save_db, inventory_id);
    if (equipment_id) m_equipment->load(save_db, equipment_id);

    return sql_id;
}

// Retrieves the location of this Mobile, in the form of a Room ID.
uint32_t Mobile::location() const { return m_location; }

// Retrieves the name of this Mobile.
std::string Mobile::name() const { return m_name; }

// Causes time to pass for this Mobile.
bool Mobile::pass_time(float seconds)
{
    // For the player, time passes in the world itself.
    if (is_player()) return core()->world()->time_weather()->pass_time(seconds);

    // For NPCs, we'll just take the time from their action timer.
    m_action_timer -= seconds;
    return true;
}

// Saves this Mobile.
uint32_t Mobile::save(std::shared_ptr<SQLite::Database> save_db)
{
    const uint32_t inventory_id = m_inventory->save(save_db);
    const uint32_t equipment_id = m_equipment->save(save_db);

    const uint32_t sql_id = core()->sql_unique_id();
    SQLite::Statement query(*save_db, "INSERT INTO mobiles ( action_timer, equipment, inventory, location, name, sql_id ) VALUES ( ?, ?, ?, ?, ?, ? )");
    if (m_action_timer) query.bind(1, m_action_timer);
    if (equipment_id) query.bind(2, equipment_id);
    if (inventory_id) query.bind(3, inventory_id);
    query.bind(4, m_location);
    if (m_name.size()) query.bind(5, m_name);
    query.bind(6, sql_id);
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

// Sets the name of this Mobile.
void Mobile::set_name(const std::string &name) { m_name = name; }
