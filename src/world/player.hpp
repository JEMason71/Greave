// world/player.hpp -- The Player class is derived from Mobile, and defines the player character in the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "world/mobile.hpp"


class Player : public Mobile
{
public:
    static const std::string    SQL_PLAYER;    // The SQL table construction string for the player data.

    void            load(std::shared_ptr<SQLite::Database> save_db, unsigned int sql_id) override;  // Loads the Player data.
    uint32_t        save(std::shared_ptr<SQLite::Database> save_db) override;   // Saves this Player.
    Mobile::Type    type() override;    // Returns Mobile::Type::Player, so we can check what type of Mobile this is when using a Mobile pointer.
};
