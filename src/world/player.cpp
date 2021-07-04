// world/player.cpp -- The Player class is derived from Mobile, and defines the player character in the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.hpp"
#include "world/world.hpp"
#include "world/player.hpp"


// The SQL table construction string for the player data.
const std::string   Player::SQL_PLAYER = "CREATE TABLE player ( awake INTEGER NOT NULL, mob_target INTEGER, sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL )";


// Constructor, sets default values.
Player::Player() : m_awake(Awake::ACTIVE), m_mob_target(0) { set_species("humanoid"); }

// Checks the consciousness level of this Mobile.
Player::Awake Player::awake() const { return m_awake; }

// Returns true if this Mobile is a Player, false if not.
bool Player::is_player() const { return true; }

// Loads the Player data.
uint32_t Player::load(std::shared_ptr<SQLite::Database> save_db, unsigned int sql_id)
{
    SQLite::Statement query(*save_db, "SELECT * FROM player");
    if (query.executeStep())
    {
        m_awake = static_cast<Awake>(query.getColumn("awake").getInt());
        if (!query.isColumnNull("mob_target")) m_mob_target = query.getColumn("mob_target").getUInt();
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
        for (unsigned int i = 0; i < core()->world()->mob_count(); i++)
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

// Saves this Player.
uint32_t Player::save(std::shared_ptr<SQLite::Database> save_db)
{
    const uint32_t sql_id = Mobile::save(save_db);
    SQLite::Statement query(*save_db, "INSERT INTO player ( awake, mob_target, sql_id ) VALUES ( ?, ?, ? )");
    query.bind(1, static_cast<int>(m_awake));
    if (m_mob_target) query.bind(2, m_mob_target);
    query.bind(3, sql_id);
    query.exec();
    return sql_id;
}

// Sets the consciousness level of this Mobile.
void Player::set_awake(Player::Awake awake) { m_awake = awake; }

// Sets a new Mobile target.
void Player::set_mob_target(uint32_t target) { m_mob_target = target; }
