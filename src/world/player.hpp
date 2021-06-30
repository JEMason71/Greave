// world/player.hpp -- The Player class is derived from Mobile, and defines the player character in the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "world/mobile.hpp"


class Player : public Mobile
{
public:
    enum class Awake : uint8_t { ACTIVE, WAITING, RESTING, SLEEPING, COMA };

    static const std::string    SQL_PLAYER; // The SQL table construction string for the player data.

                Player();                   // Constructor, sets default values.
    Awake       awake() const;              // Checks the consciousness level of the Player.
    bool        is_player() const override; // Returns true if this Mobile is a Player, false if not.
    uint32_t    load(std::shared_ptr<SQLite::Database> save_db, unsigned int sql_id) override;  // Loads the Player data.
    uint32_t    save(std::shared_ptr<SQLite::Database> save_db) override;   // Saves this Player.
    void        set_awake(Awake awake);     // Sets the consciousness level of this Player.

    Awake       m_awake;    // The consciousness of the Player character.
};
