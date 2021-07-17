// world/player.hpp -- The Player class is derived from Mobile, and defines the player character in the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"
#include "world/item.hpp"
#include "world/mobile.hpp"


class Player : public Mobile
{
public:
    static const int            BLOOD_TOX_WARNING;  // The level at which the player is warned of increasing blood toxicity.
    static const std::string    SQL_PLAYER;         // The SQL table construction string for the player data.
    static const std::string    SQL_SKILLS;         // The SQL table construction string for the player skills data.

                Player();                           // Constructor, sets default values.
    void        add_food(int power);                // Eats food, increasing the hunger counter.
    void        add_money(uint32_t amount);         // Adds money to the player's wallet.
    void        add_water(int power);               // Drinks some water, increasing the thirst counter.
    int         blood_tox() const;                  // Retrieves the player's blood toxicity level.
    int         clothes_warmth() const;             // Gets the clothing warmth level from the Player.
    std::string death_reason() const;               // Retrieves the player's death reason.
    void        gain_skill_xp(const std::string& skill_id, float xp = 1.0f);    // Gains experience in a skill.
    int         hunger() const;                     // Checks the current hunger level.
    void        hunger_tick();                      // The player gets a little more hungry.
    void        increase_tox(int power);            // Increases the player's blood toxicity.
    bool        is_dead() const override;           // Checks if this Player is dead.
    bool        is_player() const override;         // Returns true if this Mobile is a Player, false if not.
    uint32_t    load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id) override;  // Loads the Player data.
    uint32_t    max_carry() const override;         // The maximum weight the player can carry.
    uint32_t    mob_target();                       // Retrieves the Mobile target if it's still valid, or sets it to 0 if not.
    uint32_t    money() const;                      // Check how much money we're carrying.
    int         mp(bool max = false) const;         // Retrieves the MP (or maximum MP) of the player.
    void        reduce_hp(int amount, bool death_message = true) override;  // Reduces the player's hit points.
    void        reduce_mp(int amount);              // Reduces the player's mana points.
    void        reduce_sp(int amount);              // Reduces the player's stamina points.
    void        remove_money(uint32_t amount);      // Removes money from the player.
    void        restore_mp(int amount);             // Restores the player's mana points.
    void        restore_sp(int amount);             // Restores the player's stamina points.
    uint32_t    save(std::shared_ptr<SQLite::Database> save_db) override;   // Saves this Player.
    void        set_death_reason(const std::string &reason);    // Sets the reason for this Player dying.
    void        set_mob_target(uint32_t target);    // Sets a new Mobile target.
    int         skill_level(const std::string &skill_id) const; // Returns the skill level of a specified skill of this Player.
    const std::map<std::string, int>&   skill_map() const;      // Returns read-only access to the player's skill levels.
    int         sp(bool max = false) const;         // Retrieves the SP (or maximum SP) of the player.
    int         thirst() const;                     // Checks the current thirst level.
    void        thirst_tick();                      // The player gets a little more thirsty.
    void        tick_blood_tox();                   // Reduces blood toxicity.
    void        tick_hp_regen() override;           // Regenerates HP over time.
    void        tick_mp_regen();                    // Regenerates MP over time.
    void        tick_sp_regen();                    // Regenerates SP over time.
    bool        wearing_armour(ItemSub type);       // Checks if the player is wearing a certain type of armour (light/medium/heavy).

private:
    static const float  BASE_SKILL_COST_MULTIPLIER;     // The higher this number, the slower player skill levels increase.
    static const int    BASE_SKILL_COST_LEVEL_OFFSET;   // The skill XP cost formula is offset by this many levels.
    static const int    BLOOD_TOX_POISON_CHANCE;        // 1 in X chance of being poisoned by the below level of toxicity.
    static const int    BLOOD_TOX_POISON_LEVEL;         // The level at which the player can be poisoned by blood toxicity.
    static const int    BLOOD_TOX_POISON_POWER_BASE;    // The base amount of power for the blood toxicity poison debuff.
    static const int    BLOOD_TOX_POISON_POWER_RNG;     // The RNG variance additional power for the blood toxicity poison debuff.
    static const int    BLOOD_TOX_POISON_TIME_BASE;     // The base amount of time for the blood toxicity poison debuff.
    static const int    BLOOD_TOX_POISON_TIME_RNG;      // The RNG variance additional time for the blood toxicity poison debuff.
    static const int    BLOOD_TOX_VOMIT_LEVEL;          // The level at which the player can vomit from blood toxicity.
    static const int    BLOOD_TOX_VOMIT_CHANCE;         // 1 in X chance of vomiting past the above level of toxicity.
    static const int    HP_PER_TOUGHNESS;               // How many extra hit points to gain per point of toughness.
    static const int    HUNGER_MAX;                     // The maximum hunger value (when this is maxed, the player is fully satiated.)
    static const int    MP_DEFAULT;                     // THe default mana points maximum for the player.
    static const int    MP_REGEN_PER_TICK;              // How much mana is regenerated each mana heartbeat tick?
    static const int    REGEN_TIME_COST_HUNGER;         // How many hunger ticks it costs to regenerate a unit of health.
    static const int    REGEN_TIME_COST_THIRST;         // How many thirst ticks it costs to regenerate a unit of health.
    static const float  SKILL_HAULING_DIVISOR;          // This number affects how effective the Hauling skill is at increasing maximum carry weight. LOWER number = skill allows more carry weight.
    static const int    SP_DEFAULT;                     // The default stamina points maximum for the player.
    static const int    SP_REGEN_PER_TICK;              // How much stamina is regenerated each stamina heartbeat tick?
    static const int    THIRST_MAX;                     // The maximum thirst value (when this is maxed, the player is fully quenched.)
    static const float  TOUGHNESS_GAIN_MODIFIER;        // The higher this value, the faster toughness will increase in combat.

    void        recalc_max_hp();    // Recalculates maximum HP, after toughness skill gains.

    int         m_blood_tox;    // Blood toxicity level.
    std::string m_death_reason; // The cause of death, when it happens.
    uint8_t     m_hunger;       // The hunger counter. 20 = completely full, 0 = starved to death.
    uint32_t    m_mob_target;   // The last Mobile to have been attacked.
    uint32_t    m_money;        // The amount of coin the player is carrying.
    int         m_mp[2];        // The current and maximum mana points.
    std::map<std::string, int>      m_skill_levels; // The skill levels learned by this Player, if any.
    std::map<std::string, float>    m_skill_xp;     // The experience levels of skills on this Player, if any.
    int         m_sp[2];        // The current and maximum stamina points.
    uint8_t     m_thirst;       // The thirst counter. 20 = compmpletely hydrated, 0 = died of dehydration.
};
