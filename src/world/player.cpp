// world/player.cpp -- The Player class is derived from Mobile, and defines the player character in the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.hpp"
#include "world/player.hpp"


// The SQL table construction string for the player data.
const std::string   Player::SQL_PLAYER = "CREATE TABLE player ( sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL )";


// Loads the Player data.
void Player::load(std::shared_ptr<SQLite::Database> save_db, unsigned int sql_id)
{
    SQLite::Statement query(*save_db, "SELECT * FROM player");
    if (query.executeStep())
    {
        sql_id = query.getColumn("sql_id").getUInt();
    }
    else throw std::runtime_error("Could not load player data!");
    Mobile::load(save_db, sql_id);
}

// Saves this Player.
uint32_t Player::save(std::shared_ptr<SQLite::Database> save_db)
{
    const uint32_t sql_id = Mobile::save(save_db);
    SQLite::Statement query(*save_db, "INSERT INTO player ( sql_id ) VALUES ( ? )");
    query.bind(1, sql_id);
    query.exec();
    return sql_id;
}

// Returns Mobile::Type::Player, so we can check what type of Mobile this is when using a Mobile pointer.
Mobile::Type Player::type() { return Mobile::Type::PLAYER; }
