// world/player.hpp -- The Player class is derived from Mobile, and defines the player character in the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "world/mobile.hpp"


class Player : public Mobile
{
public:
    static const std::string    SQL_PLAYER; // The SQL table construction string for the player data.

                Player();                           // Constructor, sets default values.
    bool        is_player() const override;         // Returns true if this Mobile is a Player, false if not.
    uint32_t    load(std::shared_ptr<SQLite::Database> save_db, unsigned int sql_id) override;  // Loads the Player data.
    uint32_t    mob_target();                       // Retrieves the Mobile target if it's still valid, or sets it to 0 if not.
    uint32_t    save(std::shared_ptr<SQLite::Database> save_db) override;   // Saves this Player.
    void        set_mob_target(uint32_t target);    // Sets a new Mobile target.

private:
    uint32_t    m_mob_target;   // The last Mobile to have been attacked.
};
