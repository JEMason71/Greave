// world/player.hpp -- The Player class is derived from Mobile, and defines the player character in the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "world/mobile.hpp"


class Player : public Mobile
{
public:
    static const std::string    SQL_PLAYER; // The SQL table construction string for the player data.
    static const std::string    SQL_SKILLS; // The SQL table construction string for the player skills data.

                Player();                           // Constructor, sets default values.
    void        add_money(uint32_t amount);         // Adds money to the player's wallet.
    int         clothes_warmth() const;             // Gets the clothing warmth level from the Player.
    std::string death_reason() const;               // Retrieves the player's death reason.
    void        gain_skill_xp(const std::string& skill_id, float xp = 1.0f);    // Gains experience in a skill.
    bool        is_player() const override;         // Returns true if this Mobile is a Player, false if not.
    uint32_t    load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id) override;  // Loads the Player data.
    uint32_t    mob_target();                       // Retrieves the Mobile target if it's still valid, or sets it to 0 if not.
    uint32_t    money() const;                      // Check how much money we're carrying.
    void        remove_money(uint32_t amount);      // Removes money from the player.
    uint32_t    save(std::shared_ptr<SQLite::Database> save_db) override;   // Saves this Player.
    void        set_death_reason(const std::string &reason);    // Sets the reason for this Player dying.
    void        set_mob_target(uint32_t target);    // Sets a new Mobile target.
    int         skill_level(const std::string &skill_id) const; // Returns the skill level of a specified skill of this Player.

private:
    static const float  BASE_SKILL_COST_MULTIPLIER;     // The higher this number, the slower player skill levels increase.
    static const int    BASE_SKILL_COST_LEVEL_OFFSET;   // The skill XP cost formula is offset by this many levels.

    std::string m_death_reason; // The cause of death, when it happens.
    uint32_t    m_mob_target;   // The last Mobile to have been attacked.
    uint32_t    m_money;        // The amount of coin the player is carrying.
    std::map<std::string, int>      m_skill_levels; // The skill levels learned by this Player, if any.
    std::map<std::string, float>    m_skill_xp;     // The experience levels of skills on this Player, if any.
};
