// world/player.cpp -- The Player class is derived from Mobile, and defines the player character in the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.hpp"
#include "core/guru.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/world.hpp"
#include "world/player.hpp"


const int Player:: BASE_SKILL_COST_LEVEL_OFFSET =   0;      // The skill XP cost formula is offset by this many levels.
const float Player::BASE_SKILL_COST_MULTIPLIER =    2.0f;   // The higher this number, the slower player skill levels increase.

// The SQL table construction string for the player data.
const std::string Player::SQL_PLAYER = "CREATE TABLE player ( mob_target INTEGER, money INTEGER NOT NULL, sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL )";

// The SQL table construction string for the player skills data.
const std::string Player::SQL_SKILLS = "CREATE TABLE skills ( id TEXT PRIMARY KEY UNIQUE NOT NULL, level INTEGER NOT NULL, xp REAL )";


// Constructor, sets default values.
Player::Player() : m_death_reason("the will of the gods"), m_mob_target(0), m_money(0)
{
    set_species("humanoid");
    set_name("Player");
}

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

// Retrieves the player's death reason.
std::string Player::death_reason() const { return m_death_reason; }

// Gains experience in a skill.
void Player::gain_skill_xp(const std::string& skill_id, float xp)
{
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

    SQLite::Statement skill_query(*save_db, "SELECT * FROM skills");
    while (skill_query.executeStep())
    {
        const std::string skill_id = skill_query.getColumn("id").getString();
        m_skill_levels.insert(std::make_pair(skill_id, skill_query.getColumn("level").getInt()));
        if (!skill_query.isColumnNull("xp")) m_skill_xp.insert(std::make_pair(skill_id, skill_query.getColumn("xp").getDouble()));
    }

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

    for (const auto &kv : m_skill_levels)
    {
        SQLite::Statement skill_query(*save_db, "INSERT INTO skills ( id, level, xp ) VALUES ( ?, ?, ?)");
        skill_query.bind(1, kv.first);
        skill_query.bind(2, kv.second);
        const auto it = m_skill_xp.find(kv.first);
        if (it != m_skill_xp.end()) skill_query.bind(3, it->second);
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
