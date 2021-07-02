// world/mobile.cpp -- The Mobile class defines entities that can move and interact with the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.hpp"
#include "core/random.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/mobile.hpp"
#include "world/time-weather.hpp"
#include "world/world.hpp"


// The SQL table construction string for Mobiles.
const std::string   Mobile::SQL_MOBILES =   "CREATE TABLE mobiles ( action_timer REAL, equipment INTEGER UNIQUE, hp INTEGER NOT NULL, hp_max INTEGER NOT NULL, "
    "inventory INTEGER UNIQUE, location INTEGER NOT NULL, name TEXT, parser_id INTEGER, species TEXT NOT NULL, sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL )";


// Constructor, sets default values.
Mobile::Mobile() : m_action_timer(0), m_equipment(std::make_shared<Inventory>()), m_inventory(std::make_shared<Inventory>()), m_location(0), m_parser_id(0)
{
    m_hp[0] = m_hp[1] = 100;
}

// Returns the number of seconds needed for this Mobile to make an attack.
float Mobile::attack_speed() const
{
    auto main_hand = m_equipment->get(EquipSlot::HAND_MAIN);
    auto off_hand = m_equipment->get(EquipSlot::HAND_OFF);
    const bool main_can_attack = (main_hand && main_hand->type() == ItemType::WEAPON && main_hand->subtype() == ItemSub::MELEE);
    const bool off_can_attack = (off_hand && off_hand->type() == ItemType::WEAPON && off_hand->subtype() == ItemSub::MELEE);

    // Attack speed is the slowest of the equipped weapons.
    float speed = 0.0f;
    if (main_can_attack) speed = main_hand->speed();
    if (off_can_attack && off_hand->speed() > speed) speed = off_hand->speed();
    if (!main_can_attack && !off_can_attack) speed = 1.0f;

    if (!speed)
    {
        speed = 1.0f;
        throw std::runtime_error("Cannot determine attack speed for " + name() + "!");
    }

    return speed;
}

// Returns a pointer to the Movile's equipment.
const std::shared_ptr<Inventory> Mobile::equ() const { return m_equipment; }

// Retrieves the HP (or maximum HP) of this Mobile.
int Mobile::hp(bool max) const { return m_hp[max ? 1 : 0]; }

// Returns a pointer to the Mobile's Inventory.
const std::shared_ptr<Inventory> Mobile::inv() const { return m_inventory; }

// Checks if this Mobile is dead.
bool Mobile::is_dead() const { return m_hp[0] <= 0; }

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
        m_hp[0] = query.getColumn("hp").getInt();
        m_hp[1] = query.getColumn("hp_max").getInt();
        if (!query.isColumnNull("inventory")) inventory_id = query.getColumn("inventory").getUInt();
        m_location = query.getColumn("location").getUInt();
        if (!query.isColumnNull("name")) m_name = query.getColumn("name").getString();
        if (!query.isColumnNull("parser_id")) m_parser_id = query.getColumn("parser_id").getInt();
        m_species = query.getColumn("species").getString();
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

// Generates a new parser ID for this Item.
void Mobile::new_parser_id() { m_parser_id = core()->rng()->rnd(1, 9999); }

// Retrieves the current ID of this Item, for parser differentiation.
uint16_t Mobile::parser_id() const { return m_parser_id; }

// Causes time to pass for this Mobile.
bool Mobile::pass_time(float seconds)
{
    // For the player, time passes in the world itself.
    if (is_player()) return core()->world()->time_weather()->pass_time(seconds);

    // For NPCs, we'll just take the time from their action timer.
    m_action_timer -= seconds;
    return true;
}

// Reduces this Mobile's hit points.
void Mobile::reduce_hp(int amount) { m_hp[0] -= amount; }

// Restores a specified amount of hit points.
int Mobile::restore_hp(int amount)
{
    int missing = m_hp[1] - m_hp[0];
    if (missing < amount) amount = missing;
    m_hp[0] += missing;
    return missing;
}

// Saves this Mobile.
uint32_t Mobile::save(std::shared_ptr<SQLite::Database> save_db)
{
    const uint32_t inventory_id = m_inventory->save(save_db);
    const uint32_t equipment_id = m_equipment->save(save_db);

    const uint32_t sql_id = core()->sql_unique_id();
    SQLite::Statement query(*save_db, "INSERT INTO mobiles ( action_timer, equipment, hp, hp_max, inventory, location, name, parser_id, species, sql_id ) "
        "VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )");
    if (m_action_timer) query.bind(1, m_action_timer);
    if (equipment_id) query.bind(2, equipment_id);
    query.bind(3, m_hp[0]);
    query.bind(4, m_hp[1]);
    if (inventory_id) query.bind(5, inventory_id);
    query.bind(6, m_location);
    if (m_name.size()) query.bind(7, m_name);
    if (m_parser_id) query.bind(8, m_parser_id);
    query.bind(9, m_species);
    query.bind(10, sql_id);
    query.exec();
    return sql_id;
}

// Sets the current (and, optionally, maximum) HP of this Mobile.
void Mobile::set_hp(int hp, int hp_max)
{
    m_hp[0] = hp;
    if (hp_max) m_hp[1] = hp_max;
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

// Sets the species of this Mobile.
void Mobile::set_species(const std::string &species) { m_species = species; }

// Checks the species of this Mobile.
std::string Mobile::species() const { return m_species; }
