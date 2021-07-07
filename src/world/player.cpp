// world/player.cpp -- The Player class is derived from Mobile, and defines the player character in the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.hpp"
#include "core/guru.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/world.hpp"
#include "world/player.hpp"


// The SQL table construction string for the player data.
const std::string   Player::SQL_PLAYER = "CREATE TABLE player ( mob_target INTEGER, money INTEGER NOT NULL, sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL )";


// Constructor, sets default values.
Player::Player() : m_mob_target(0), m_money(0) { set_species("humanoid"); }

// Adds money to the player's wallet.
void Player::add_money(uint32_t amount)
{
    // Avoid integer overflow.
    if (m_money + amount < m_money)
    {
        core()->guru()->nonfatal("Intercepted money integer overflow!", Guru::WARN);
        m_money = UINT32_MAX;
    }
    else m_money += amount;
}

// Gets the clothing warmth level from the Player.
int Player::clothes_warmth() const
{
    int warmth = 0;
    for (size_t i = 0; i < m_equipment->count(); i++)
        warmth += m_equipment->get(i)->warmth();
    return warmth;
}

// Returns true if this Mobile is a Player, false if not.
bool Player::is_player() const { return true; }

// Loads the Player data.
uint32_t Player::load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id)
{
    SQLite::Statement query(*save_db, "SELECT * FROM player");
    if (query.executeStep())
    {
        if (!query.isColumnNull("mob_target")) m_mob_target = query.getColumn("mob_target").getUInt();
        m_money = query.getColumn("money").getUInt();
        sql_id = query.getColumn("sql_id").getUInt();
    }
    else throw std::runtime_error("Could not load player data!");
    return Mobile::load(save_db, sql_id);
}

// Retrieves the Mobile target if it's still valid, or sets it to 0 if not.
uint32_t Player::mob_target()
{
    if (m_mob_target)
    {
        for (size_t i = 0; i < core()->world()->mob_count(); i++)
        {
            const auto mob = core()->world()->mob_vec(i);
            if (mob->id() == m_mob_target)
            {
                if (mob->location() == m_location) return m_mob_target;
                else break;
            }
        }
        m_mob_target = 0;   // If we couldn't make a match, or the matched Mobile is no longer here, just clear the target.
    }
    return m_mob_target;
}

// Check how much money we're carrying.
uint32_t Player::money() const { return m_money; }

// Removes money from the player.
void Player::remove_money(uint32_t amount)
{
    if (amount > m_money)
    {
        m_money = 0;
        core()->guru()->nonfatal("Attempt to remove more money than the player owns!", Guru::ERROR);
    }
    else m_money -= amount;
}

// Saves this Player.
uint32_t Player::save(std::shared_ptr<SQLite::Database> save_db)
{
    const uint32_t sql_id = Mobile::save(save_db);
    SQLite::Statement query(*save_db, "INSERT INTO player ( mob_target, money, sql_id ) VALUES ( ?, ?, ? )");
    if (m_mob_target) query.bind(1, m_mob_target);
    query.bind(2, m_money);
    query.bind(3, sql_id);
    query.exec();
    return sql_id;
}

// Sets a new Mobile target.
void Player::set_mob_target(uint32_t target) { m_mob_target = target; }
