// world/player.cpp -- The Player class is derived from Mobile, and defines the player character in the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "actions/eat-drink.hpp"
#include "core/core.hpp"
#include "core/guru.hpp"
#include "core/random.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/world.hpp"
#include "world/player.hpp"
#include "world/time-weather.hpp"


const int   Player::BASE_SKILL_COST_LEVEL_OFFSET =  0;      // The skill XP cost formula is offset by this many levels.
const float Player::BASE_SKILL_COST_MULTIPLIER =    2.0f;   // The higher this number, the slower player skill levels increase.
const int   Player::BLOOD_TOX_POISON_CHANCE =       3;      // 1 in X chance of being poisoned by the below level of toxicity.
const int   Player::BLOOD_TOX_POISON_LEVEL =        10;     // The level at which the player can be poisoned by blood toxicity.
const int   Player::BLOOD_TOX_POISON_POWER_BASE =   5;      // The base amount of power for the blood toxicity poison debuff.
const int   Player::BLOOD_TOX_POISON_POWER_RNG =    10;     // The RNG variance additional power for the blood toxicity poison debuff.
const int   Player::BLOOD_TOX_POISON_TIME_BASE =    5;      // The base amount of time for the blood toxicity poison debuff.
const int   Player::BLOOD_TOX_POISON_TIME_RNG =     5;      // The RNG variance additional time for the blood toxicity poison debuff.
const int   Player::BLOOD_TOX_VOMIT_LEVEL =         6;      // The level at which the player can vomit from blood toxicity.
const int   Player::BLOOD_TOX_VOMIT_CHANCE =        4;      // 1 in X chance of vomiting past the above level of toxicity.
const int   Player::BLOOD_TOX_WARNING =             4;      // The level at which the player is warned of increasing blood toxicity.
const int   Player::HUNGER_MAX =                    20;     // The maximum hunger value (when this is maxed, the player is fully satiated.)
const int   Player::REGEN_TIME_COST_HUNGER =        60;     // How many hunger ticks it costs to regenerate a unit of health.
const int   Player::REGEN_TIME_COST_THIRST =        30;     // How many thirst ticks it costs to regenerate a unit of health.
const float Player::SKILL_HAULING_DIVISOR =         50;     // This number affects how effective the Hauling skill is at increasing maximum carry weight. LOWER number = skill allows more carry weight.
const int   Player::SP_DEFAULT =                    100;    // The default stamina points maximum for the player.
const int   Player::THIRST_MAX =                    20;     // The maximum thirst value (when this is maxed, the player is fully quenched.)

// The SQL table construction string for the player data.
const std::string Player::SQL_PLAYER = "CREATE TABLE player ( blood_tox INTEGER, hunger INTEGER NOT NULL, mob_target INTEGER, money INTEGER NOT NULL, sp INTEGER NOT NULL, sp_max INTEGER NOT NULL, sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL, thirst INTEGET NOT NULL )";

// The SQL table construction string for the player skills data.
const std::string Player::SQL_SKILLS = "CREATE TABLE skills ( id TEXT PRIMARY KEY UNIQUE NOT NULL, level INTEGER NOT NULL, xp REAL )";


// Constructor, sets default values.
Player::Player() : m_blood_tox(0), m_death_reason("the will of the gods"), m_hunger(HUNGER_MAX), m_mob_target(0), m_money(0), m_thirst(THIRST_MAX)
{
    set_species("humanoid");
    set_name("Player");
    m_sp[0] = m_sp[1] = SP_DEFAULT;
}

// Eats food, increasing the hunger counter.
void Player::add_food(int power) { m_hunger += power; }

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

// Drinks some water, increasing the thirst counter.
void Player::add_water(int power)
{
    m_thirst += power;
    if (m_thirst > THIRST_MAX) m_thirst = THIRST_MAX;
}

// Retrieves the player's blood toxicity level.
int Player::blood_tox() const { return m_blood_tox; }

// Gets the clothing warmth level from the Player.
int Player::clothes_warmth() const
{
    int warmth = 0;
    for (size_t i = 0; i < m_equipment->count(); i++)
        warmth += m_equipment->get(i)->warmth();
    return warmth;
}

// Retrieves the player's death reason.
std::string Player::death_reason() const { return m_death_reason; }

// Gains experience in a skill.
void Player::gain_skill_xp(const std::string& skill_id, float xp)
{
    if (is_dead()) return;
    xp *= core()->world()->get_skill_multiplier(skill_id);
    if (xp <= 0)
    {
        if (xp < 0) core()->guru()->nonfatal("Attempt to give negative XP in " + skill_id, Guru::WARN);
        return;
    }
    auto it = m_skill_xp.find(skill_id);
    if (it == m_skill_xp.end())
    {
        m_skill_xp.insert(std::pair<std::string, float>(skill_id, xp));
        it = m_skill_xp.find(skill_id); // We'll need to refer to this in the level-up code below.
    }
    else it->second += xp;

    int current_level = skill_level(skill_id);
    if (!current_level) m_skill_levels.insert(std::pair<std::string, int>(skill_id, 0));
    bool level_increased = false;
    while (it->second > 0)
    {
        const float xp_to_next_level = BASE_SKILL_COST_MULTIPLIER * std::pow(current_level + BASE_SKILL_COST_LEVEL_OFFSET, 2);
        if (it->second >= xp_to_next_level)
        {
            it->second -= xp_to_next_level;
            current_level++;
            level_increased = true;
            m_skill_levels.find(skill_id)->second = current_level;
        }
        else break;
    }
    if (level_increased) core()->message("{G}Your skill in {C}" + core()->world()->get_skill_name(skill_id) + " {G}has increased to {C}" + std::to_string(current_level) + "{G}!");
}

// Checks the current hunger level.
int Player::hunger() const { return m_hunger; }

// The player gets a little more hungry.
void Player::hunger_tick()
{
    switch (--m_hunger)
    {
        case 0:
            core()->message("{y}You collapse from {Y}starvation{y}, too weak to keep going.");
            set_death_reason("starved to death");
            break;
        case 1: case 2: core()->message("{Y}You are starving to death!"); break;
        case 3: case 4: core()->message("{Y}You almost collapse from the hunger pain!"); break;
        case 5: case 6: core()->message("{Y}You are desperately hungry!"); break;
        case 7: case 8: core()->message("{Y}You are ravenously hungry!"); break;
        case 9: case 10: core()->message("{y}Your stomach rumbles loudly!"); break;
        case 11: case 12: core()->message("{y}Your stomach rumbles quietly."); break;
        case 14: core()->message("{y}You're starting to feel peckish."); break;
    }
}

// Increases the player's blood toxicity.
void Player::increase_tox(int power)
{
    const auto rng = core()->rng();
    int old_tox = m_blood_tox;
    m_blood_tox += power;
    if (m_blood_tox >= BLOOD_TOX_POISON_LEVEL && rng->rnd(BLOOD_TOX_POISON_CHANCE) == 1)
    {
        set_buff(Buff::Type::POISON, rng->rnd(BLOOD_TOX_POISON_TIME_RNG) + BLOOD_TOX_POISON_TIME_BASE, rng->rnd(BLOOD_TOX_POISON_POWER_RNG) + BLOOD_TOX_POISON_POWER_BASE, true, true);
        core()->message("{G}You feel deathly ill, your stomach churning violently!");
        return;
    }
    if (m_blood_tox >= BLOOD_TOX_VOMIT_LEVEL && rng->rnd(BLOOD_TOX_VOMIT_CHANCE) == 1)
    {
        ActionEatDrink::vomit(true);
        return;
    }
    if (old_tox < BLOOD_TOX_WARNING && m_blood_tox >= BLOOD_TOX_WARNING) core()->message("{g}Your stomach churns and you feel horrible.");
}

// Checks if this Player is dead.
bool Player::is_dead() const
{
    if (m_hunger < 1 || m_thirst < 1) return true;
    return Mobile::is_dead();
}

// Returns true if this Mobile is a Player, false if not.
bool Player::is_player() const { return true; }

// Loads the Player data.
uint32_t Player::load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id)
{
    SQLite::Statement query(*save_db, "SELECT * FROM player");
    if (query.executeStep())
    {
        if (!query.isColumnNull("blood_tox")) m_blood_tox = query.getColumn("blood_tox").getInt();
        m_hunger = query.getColumn("hunger").getInt();
        if (!query.isColumnNull("mob_target")) m_mob_target = query.getColumn("mob_target").getUInt();
        m_money = query.getColumn("money").getUInt();
        m_sp[0] = query.getColumn("sp").getInt();
        m_sp[1] = query.getColumn("sp_max").getInt();
        sql_id = query.getColumn("sql_id").getUInt();
        m_thirst = query.getColumn("thirst").getInt();
    }
    else throw std::runtime_error("Could not load player data!");

    SQLite::Statement skill_query(*save_db, "SELECT * FROM skills");
    while (skill_query.executeStep())
    {
        const std::string skill_id = skill_query.getColumn("id").getString();
        m_skill_levels.insert(std::make_pair(skill_id, skill_query.getColumn("level").getInt()));
        if (!skill_query.isColumnNull("xp")) m_skill_xp.insert(std::make_pair(skill_id, skill_query.getColumn("xp").getDouble()));
    }

    return Mobile::load(save_db, sql_id);
}

// The maximum weight the player can carry.
uint32_t Player::max_carry() const
{
    const uint32_t base_carry = Mobile::max_carry();
    return base_carry + std::round(base_carry * (static_cast<float>(skill_level("HAULING")) / SKILL_HAULING_DIVISOR));
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

// Reduces the player's hit points.
void Player::reduce_hp(int amount, bool death_message)
{
    if (amount >= m_hp[0] && tag(MobileTag::ArenaFighter)) core()->message("{m}The last thing you hear as your lifeless body hits the ground is the sadistic cheering of the crowd and the victorious yell of your opponent.");
    Mobile::reduce_hp(amount, death_message);
}

// Reduces the player's stamina points.
void Player::reduce_sp(int amount)
{
    if (amount <= 0) return;
    if (amount > m_sp[0]) m_sp[0] = 0;
    else m_sp[0] -= amount;
}

// Restores the player's stamina points.
void Player::restore_sp(int amount)
{
    if (amount <= 0) return;
    if (amount + m_sp[0] >= m_sp[1]) m_sp[0] = m_sp[1];
    else m_sp[0] += amount;
}

// Saves this Player.
uint32_t Player::save(std::shared_ptr<SQLite::Database> save_db)
{
    const uint32_t sql_id = Mobile::save(save_db);
    SQLite::Statement query(*save_db, "INSERT INTO player ( blood_tox, hunger, mob_target, money, sp, sp_max, sql_id, thirst ) VALUES ( :blood_tox, :hunger, :mob_target, :money, :sp, :sp_max, :sql_id, :thirst )");
    if (m_blood_tox) query.bind(":blood_tox", m_blood_tox);
    query.bind(":hunger", m_hunger);
    if (m_mob_target) query.bind(":mob_target", m_mob_target);
    query.bind(":money", m_money);
    query.bind(":sp", m_sp[0]);
    query.bind(":sp_max", m_sp[1]);
    query.bind(":sql_id", sql_id);
    query.bind(":thirst", m_thirst);
    query.exec();

    for (const auto &kv : m_skill_levels)
    {
        SQLite::Statement skill_query(*save_db, "INSERT INTO skills ( id, level, xp ) VALUES ( :id, :level, :xp )");
        skill_query.bind(":id", kv.first);
        skill_query.bind(":level", kv.second);
        const auto it = m_skill_xp.find(kv.first);
        if (it != m_skill_xp.end()) skill_query.bind(":xp", it->second);
        skill_query.exec();
    }

    return sql_id;
}

// Sets the reason for this Player dying.
void Player::set_death_reason(const std::string &reason) { m_death_reason = reason; }

// Sets a new Mobile target.
void Player::set_mob_target(uint32_t target) { m_mob_target = target; }

// Returns the skill level of a specified skill of this Player.
int Player::skill_level(const std::string &skill_id) const
{
    const auto it = m_skill_levels.find(skill_id);
    if (it == m_skill_levels.end()) return 0;
    else return it->second;
}

// Returns read-only access to the player's skill levels.
const std::map<std::string, int>& Player::skill_map() const { return m_skill_levels; }

// Retrieves the SP (or maximum SP) of the player.
int Player::sp(bool max) const { return m_sp[max ? 1 : 0]; }

// Checks the current thirst level.
int Player::thirst() const { return m_thirst; }

// The player gets a little more thirsty.
void Player::thirst_tick()
{
    switch (--m_thirst)
    {
        case 0:
            core()->message("{u}You collapse from {U}severe dehydration{u}.");
            set_death_reason("died from dehydration");
            break;
        case 1: case 2: core()->message("{U}You are dying of dehydration!"); break;
        case 3: case 4: core()->message("{U}Your throat is so parched it's painful!"); break;
        case 5: case 6: core()->message("{U}You are desperately thirsty!"); break;
        case 7: case 8: core()->message("{U}You are extremely thirsty!"); break;
        case 9: case 10: core()->message("{u}Your mouth feels very dry."); break;
        case 11: case 12: core()->message("{u}You really want something to drink."); break;
        case 14: core()->message("{u}You're starting to feel a little thirsty."); break;
    }
}

// Reduces blood toxicity.
void Player::tick_blood_tox() { if (m_blood_tox > 0) m_blood_tox--; }

// Regenerates HP over time.
void Player::tick_hp_regen()
{
    if (m_hp[0] < m_hp[1])
    {
        core()->world()->time_weather()->increase_heartbeat(TimeWeather::Heartbeat::HUNGER, REGEN_TIME_COST_HUNGER);
        core()->world()->time_weather()->increase_heartbeat(TimeWeather::Heartbeat::THIRST, REGEN_TIME_COST_THIRST);
    }
    Mobile::tick_hp_regen();
}
