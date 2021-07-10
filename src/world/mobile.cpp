// world/mobile.cpp -- The Mobile class defines entities that can move and interact with the game world.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "actions/combat/combat.hpp"
#include "core/core.hpp"
#include "core/guru.hpp"
#include "core/random.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/player.hpp"
#include "world/room.hpp"
#include "world/time-weather.hpp"
#include "world/world.hpp"


const float Mobile::ACTION_TIMER_CAP_MAX =                  3600.0f;    // The maximum value the action timer can ever reach.
const int   Mobile::BASE_CARRY_WEIGHT =                     30000;      // The maximum amount of weight a Mobile can carry, before modifiers.
const int   Mobile::SCAR_BLEED_INTENSITY_FROM_BLEED_TICK =  1;          // Blood type scar intensity caused by each tick of the player or an NPC bleeding.

// Flags for the name() function.
const int Mobile::NAME_FLAG_A =                 1;  // Precede the Mobile's name with 'a' or 'an', unless the name is a proper noun.
const int Mobile::NAME_FLAG_CAPITALIZE_FIRST =  2;  // Capitalize the first letter of the Mobile's name (including the "The") if set.
const int Mobile::NAME_FLAG_NO_COLOUR =         4;  // Strip colour codes from the name.
const int Mobile::NAME_FLAG_PLURAL =            8;  // Return a plural of the Mobile's name (e.g. apple -> apples).
const int Mobile::NAME_FLAG_POSSESSIVE =        16; // Change the Mobile's name to a possessive noun (e.g. goblin -> goblin's).
const int Mobile::NAME_FLAG_THE =               32; // Precede the Mobile's name with 'the', unless the name is a proper noun.

// The SQL table construction string for Buffs.
const std::string   Buff::SQL_BUFFS =       "CREATE TABLE buffs ( owner INTEGER, power INTEGER, sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL, time INTEGER, type INTEGER NOT NULL )";

// The SQL table construction string for Mobiles.
const std::string   Mobile::SQL_MOBILES =   "CREATE TABLE mobiles ( action_timer REAL, equipment INTEGER UNIQUE, gender INTEGER, hostility TEXT, hp INTEGER NOT NULL, "
    "hp_max INTEGER NOT NULL, id INTEGER UNIQUE NOT NULL, inventory INTEGER UNIQUE, location INTEGER NOT NULL, metadata TEXT, name TEXT, parser_id INTEGER, score INTEGER, "
    "spawn_room INTEGER, species TEXT NOT NULL, sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL, stance INTEGER, tags TEXT )";


// Loads this Buff from a save file.
std::shared_ptr<Buff> Buff::load(SQLite::Statement &query)
{
    auto new_buff = std::make_shared<Buff>();
    if (!query.isColumnNull("power")) new_buff->power = query.getColumn("power").getUInt();
    if (!query.isColumnNull("time")) new_buff->time = query.getColumn("time").getUInt();
    new_buff->type = static_cast<Buff::Type>(query.getColumn("type").getUInt());
    return new_buff;
}

// Saves this Buff to a save file.
void Buff::save(std::shared_ptr<SQLite::Database> save_db, uint32_t owner_id)
{
    SQLite::Statement query(*save_db, "INSERT INTO BUFFS ( owner, power, sql_id, time, type ) VALUES ( ?, ?, ?, ?, ? )");
    query.bind(1, owner_id);
    if (power) query.bind(2, power);
    query.bind(3, core()->sql_unique_id());
    if (time != USHRT_MAX) query.bind(4, time);
    query.bind(5, static_cast<int>(type));
    query.exec();
}


// Constructor, sets default values.
Mobile::Mobile() : m_action_timer(0), m_equipment(std::make_shared<Inventory>()), m_gender(Gender::IT), m_id(0), m_inventory(std::make_shared<Inventory>()), m_location(0),
    m_parser_id(0), m_score(0), m_spawn_room(0), m_stance(CombatStance::BALANCED)
{
    m_hp[0] = m_hp[1] = 100;
}

// Adds a Mobile (or the player, with ID 0) to this Mobile's hostility list.
void Mobile::add_hostility(uint32_t mob_id)
{
    // Check if this Mobile is already on the hostility vector.
    for (auto h : m_hostility)
        if (h == mob_id) return;

    // If not, add 'em to the list!
    m_hostility.push_back(mob_id);
}

// Adds a second to this Mobile's action timer.
void Mobile::add_second() { if (++m_action_timer > ACTION_TIMER_CAP_MAX) m_action_timer = ACTION_TIMER_CAP_MAX; }

// Adds to this Mobile's score.
void Mobile::add_score(int score) { m_score += score; }

// Returns the number of seconds needed for this Mobile to make an attack.
float Mobile::attack_speed() const
{
    auto main_hand = m_equipment->get(EquipSlot::HAND_MAIN);
    auto off_hand = m_equipment->get(EquipSlot::HAND_OFF);
    const bool main_can_attack = (main_hand && main_hand->type() == ItemType::WEAPON && main_hand->subtype() == ItemSub::MELEE);
    const bool off_can_attack = (off_hand && off_hand->type() == ItemType::WEAPON && off_hand->subtype() == ItemSub::MELEE);

    // Attack speed is the slowest of the equipped weapons.
    float speed = 0.0f;
    if (main_can_attack) speed = main_hand->speed();
    if (off_can_attack && off_hand->speed() > speed) speed = off_hand->speed();
    if (!main_can_attack && !off_can_attack) speed = 1.0f;

    if (!speed)
    {
        speed = 1.0f;
        throw std::runtime_error("Cannot determine attack speed for " + name() + "!");
    }

    return speed * Combat::BASE_ATTACK_SPEED_MULTIPLIER;
}

// Returns the modified chance to block for this Mobile, based on equipped gear.
float Mobile::block_mod() const
{
    float mod_perc = 100.0f;
    for (size_t i = 0; i < m_equipment->count(); i++)
        mod_perc += m_equipment->get(i)->block_mod();
    return mod_perc / 100.0f;
}

// Returns a pointer to a specified Buff.
std::shared_ptr<Buff> Mobile::buff(Buff::Type type) const
{
    for (auto b : m_buffs)
        if (b->type == type) return b;
    return nullptr;
}

// Returns the power level of the specified buff/debuff.
uint32_t Mobile::buff_power(Buff::Type type) const
{
    auto b = buff(type);
    if (b) return b->power;
    else return 0;
}

// Returns the time remaining for the specifieid buff/debuff.
uint16_t Mobile::buff_time(Buff::Type type) const
{
    auto b = buff(type);
    if (b) return b->time;
    else return 0;
}

// Checks if this Mobile has enough action timer built up to perform an action.
bool Mobile::can_perform_action(float time) const { return m_action_timer >= time; }

// Checks how much weight this Mobile is carrying.
uint32_t Mobile::carry_weight() const
{
    uint32_t total_weight = 0;
    for (size_t i = 0; i < m_inventory->count(); i++)
        total_weight += m_inventory->get(i)->weight();
    for (size_t i = 0; i < m_equipment->count(); i++)
        total_weight += m_equipment->get(i)->weight();
    return total_weight;
}

// Clears a specified buff/debuff from the Actor, if it exists.
void Mobile::clear_buff(Buff::Type type)
{
    for (size_t i = 0; i < m_buffs.size(); i++)
    {
        if (m_buffs.at(i)->type == type)
        {
            m_buffs.erase(m_buffs.begin() + i);
            return;
        }
    }
}

// Clears a metatag from an Mobile. Use with caution!
void Mobile::clear_meta(const std::string &key) { m_metadata.erase(key); }

// Clears a MobileTag from this Mobile.
void Mobile::clear_tag(MobileTag the_tag)
{
    if (!(m_tags.count(the_tag) > 0)) return;
    m_tags.erase(the_tag);
}

// Returns the modified chance to dodge for this Mobile, based on equipped gear.
float Mobile::dodge_mod() const
{
    float mod_perc = 100.0f;
    for (size_t i = 0; i < m_equipment->count(); i++)
        mod_perc += m_equipment->get(i)->dodge_mod();
    return mod_perc / 100.0f;
}

// Returns a pointer to the Movile's equipment.
const std::shared_ptr<Inventory> Mobile::equ() const { return m_equipment; }

// Retrieves the anatomy vector for this Mobile.
const std::vector<std::shared_ptr<BodyPart>>& Mobile::get_anatomy() const { return core()->world()->get_anatomy(m_species); }

// Checks if this Actor has the specified buff/debuff active.
bool Mobile::has_buff(Buff::Type type) const
{
    for (auto b : m_buffs)
        if (b->type == type) return true;
    return false;
}

// Returns a gender string (he/she/it/they/etc.)
std::string Mobile::he_she() const
{
    switch (m_gender)
    {
        case Gender::FEMALE: return "she";
        case Gender::MALE: return "he";
        case Gender::IT: return "it";
        case Gender::THEY: return "they";
        default: return "it";
    }
}

// Returns a gender string (his/her/its/their/etc.)
std::string Mobile::his_her() const
{
    switch (m_gender)
    {
        case Gender::FEMALE: return "her";
        case Gender::MALE: return "his";
        case Gender::IT: return "its";
        case Gender::THEY: return "their";
        default: return "its";
    }
}

// Returns the hostility vector.
const std::vector<uint32_t>& Mobile::hostility_vector() const { return m_hostility; }

// Retrieves the HP (or maximum HP) of this Mobile.
int Mobile::hp(bool max) const { return m_hp[max ? 1 : 0]; }

// Retrieves the unique ID of this Mobile.
uint32_t Mobile::id() const { return m_id; }

// Returns a pointer to the Mobile's Inventory.
const std::shared_ptr<Inventory> Mobile::inv() const { return m_inventory; }

// Checks if this Mobile is dead.
bool Mobile::is_dead() const { return m_hp[0] <= 0; }

// Is this Mobile hostile to the player?
bool Mobile::is_hostile() const
{
    if (tag(MobileTag::AggroOnSight)) return true;
    for (auto h : m_hostility)
        if (h == 0) return true;
    return false;
}

// Returns true if this Mobile is a Player, false if not.
bool Mobile::is_player() const { return false; }

// Loads a Mobile.
uint32_t Mobile::load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id)
{
    uint32_t inventory_id = 0, equipment_id = 0;
    SQLite::Statement query(*save_db, "SELECT * FROM mobiles WHERE sql_id = ?");
    query.bind(1, sql_id);
    if (query.executeStep())
    {
        if (!query.isColumnNull("action_timer")) m_action_timer = query.getColumn("action_timer").getDouble();
        if (!query.isColumnNull("equipment")) equipment_id = query.getColumn("equipment").getUInt();
        if (!query.isColumnNull("gender")) m_gender = static_cast<Gender>(query.getColumn("gender").getInt());
        if (!query.isColumnNull("hostility")) m_hostility = StrX::stoi_vec(StrX::string_explode(query.getColumn("hostility").getString(), " "));
        m_hp[0] = query.getColumn("hp").getInt();
        m_hp[1] = query.getColumn("hp_max").getInt();
        m_id = query.getColumn("id").getUInt();
        if (!query.isColumnNull("inventory")) inventory_id = query.getColumn("inventory").getUInt();
        m_location = query.getColumn("location").getUInt();
        if (!query.getColumn("metadata").isNull()) StrX::string_to_metadata(query.getColumn("metadata").getString(), m_metadata);
        if (!query.isColumnNull("name")) m_name = query.getColumn("name").getString();
        if (!query.isColumnNull("parser_id")) m_parser_id = query.getColumn("parser_id").getInt();
        if (!query.isColumnNull("score")) m_score = query.getColumn("score").getUInt();
        if (!query.isColumnNull("spawn_room")) m_spawn_room = query.getColumn("spawn_room").getUInt();
        m_species = query.getColumn("species").getString();
        if (!query.isColumnNull("stance")) m_stance = static_cast<CombatStance>(query.getColumn("stance").getInt());
        if (!query.isColumnNull("tags")) StrX::string_to_tags(query.getColumn("tags").getString(), m_tags);
    }
    else throw std::runtime_error("Could not load mobile data!");

    if (inventory_id) m_inventory->load(save_db, inventory_id);
    if (equipment_id) m_equipment->load(save_db, equipment_id);

    // Load any and all buffs/debuffs.
    SQLite::Statement buff_query(*save_db, "SELECT * FROM buffs WHERE owner = ?");
    buff_query.bind(1, sql_id);
    while (buff_query.executeStep())
        m_buffs.push_back(Buff::load(buff_query));

    return sql_id;
}

// Retrieves the location of this Mobile, in the form of a Room ID.
uint32_t Mobile::location() const { return m_location; }

// The maximum weight this Mobile can carry.
uint32_t Mobile::max_carry() const { return BASE_CARRY_WEIGHT; }

// Retrieves Mobile metadata.
std::string Mobile::meta(const std::string &key) const
{
    if (m_metadata.find(key) == m_metadata.end()) return "";
    std::string result = m_metadata.at(key);
    StrX::find_and_replace(result, "_", " ");
    return result;
}

// Retrieves metadata, in float format.
float Mobile::meta_float(const std::string &key) const
{
    const std::string key_str = meta(key);
    if (!key_str.size()) return 0.0f;
    else return std::stof(key_str);
}

// Retrieves metadata, in int format.
int Mobile::meta_int(const std::string &key) const
{
    const std::string key_str = meta(key);
    if (!key_str.size()) return 0;
    else return std::stoi(key_str);
}

// Retrieves metadata, in unsigned 32-bit integer format.
uint32_t Mobile::meta_uint(const std::string &key) const
{
    const std::string key_str = meta(key);
    if (!key_str.size()) return 0;
    else return std::stoul(key_str);
}

// Accesses the metadata map directly. Use with caution!
std::map<std::string, std::string>* Mobile::meta_raw() { return &m_metadata; }

// Retrieves the name of this Mobile.
std::string Mobile::name(int flags) const
{
    if (!m_name.size()) return "";
    const bool a = ((flags & Mobile::NAME_FLAG_A) == Mobile::NAME_FLAG_A);
    const bool the = ((flags & Mobile::NAME_FLAG_THE) == Mobile::NAME_FLAG_THE);
    const bool capitalize_first = ((flags & Mobile::NAME_FLAG_CAPITALIZE_FIRST) == Mobile::NAME_FLAG_CAPITALIZE_FIRST);
    const bool possessive = ((flags & Mobile::NAME_FLAG_POSSESSIVE) == Mobile::NAME_FLAG_POSSESSIVE);
    const bool plural = ((flags & Mobile::NAME_FLAG_PLURAL) == Mobile::NAME_FLAG_PLURAL);
    const bool no_colour = ((flags & Mobile::NAME_FLAG_NO_COLOUR) == Mobile::NAME_FLAG_NO_COLOUR);

    std::string ret = m_name;
    if (the && !tag(MobileTag::ProperNoun)) ret = "the " + m_name;
    else if (a && !tag(MobileTag::ProperNoun))
    {
        if (StrX::is_vowel(m_name.at(0))) ret = "an " + m_name;
        else ret = "a " + m_name;
    }
    if (capitalize_first && ret[0] >= 'a' && ret[0] <= 'z') ret[0] -= 32;
    if (possessive)
    {
        if (ret.back() == 's') ret += "'";
        else ret += "'s";
    }
    else if (plural && ret.back() != 's' && !tag(MobileTag::PluralName)) ret += "s";
    if (no_colour) ret = StrX::strip_ansi(ret);
    return ret;
}

// Generates a new parser ID for this Item.
void Mobile::new_parser_id() { m_parser_id = core()->rng()->rnd(1, 9999); }

// Returns the modified chance to parry for this Mobile, based on equipped gear.
float Mobile::parry_mod() const
{
    float mod_perc = 100.0f;
    for (size_t i = 0; i < m_equipment->count(); i++)
        mod_perc += m_equipment->get(i)->parry_mod();
    return mod_perc / 100.0f;
}

// Retrieves the current ID of this Item, for parser differentiation.
uint16_t Mobile::parser_id() const { return m_parser_id; }

// Causes time to pass for this Mobile.
bool Mobile::pass_time(float seconds, bool interruptable)
{
    // For the player, time passes in the world itself.
    if (is_player())
    {
        if (!seconds) core()->guru()->nonfatal("Attempt to pass 0 seconds on player character.", Guru::WARN);
        return core()->world()->time_weather()->pass_time(seconds, interruptable);
    }

    // For NPCs, any action clears their action timer.
    m_action_timer = 0;
    return true;
}

// Reduces this Mobile's hit points.
void Mobile::reduce_hp(int amount, bool death_message)
{
    m_hp[0] -= amount;
    if (is_player()) return;                // The player character's death is handled elsewhere.
    clear_buff(Buff::Type::RECENTLY_FLED);  // Cowardly NPCs fleeing in fear should be able to flee again when taking damage.
    if (m_hp[0] > 0) return;                // Everything below this point deals with the Mobile dying.

    if (death_message && m_location == core()->world()->player()->location())
    {
        std::string death_message = "{U}" + name(NAME_FLAG_CAPITALIZE_FIRST | NAME_FLAG_THE);
        if (tag(MobileTag::Unliving)) death_message += " is destroyed!";
        else death_message += " is slain!";
        core()->message(death_message);
    }
    core()->world()->player()->add_score(m_score);
    if (m_spawn_room) core()->world()->get_room(m_spawn_room)->clear_tag(RoomTag::MobSpawned);
    core()->world()->remove_mobile(m_id);
}

// Restores a specified amount of hit points.
int Mobile::restore_hp(int amount)
{
    int missing = m_hp[1] - m_hp[0];
    if (missing < amount) amount = missing;
    m_hp[0] += missing;
    return missing;
}

// Saves this Mobile.
uint32_t Mobile::save(std::shared_ptr<SQLite::Database> save_db)
{
    const uint32_t inventory_id = m_inventory->save(save_db);
    const uint32_t equipment_id = m_equipment->save(save_db);

    const uint32_t sql_id = core()->sql_unique_id();
    SQLite::Statement query(*save_db, "INSERT INTO mobiles ( action_timer, equipment, gender, hostility, hp, hp_max, id, inventory, location, metadata, name, parser_id, score, "
        "spawn_room, species, sql_id, stance, tags ) VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )");
    if (m_action_timer) query.bind(1, m_action_timer);
    if (equipment_id) query.bind(2, equipment_id);
    if (m_gender != Gender::IT) query.bind(3, static_cast<int>(m_gender));
    if (m_hostility.size()) query.bind(4, StrX::collapse_vector(m_hostility));
    query.bind(5, m_hp[0]);
    query.bind(6, m_hp[1]);
    query.bind(7, m_id);
    if (inventory_id) query.bind(8, inventory_id);
    query.bind(9, m_location);
    if (m_metadata.size()) query.bind(10, StrX::metadata_to_string(m_metadata));
    if (m_name.size()) query.bind(11, m_name);
    if (m_parser_id) query.bind(12, m_parser_id);
    if (m_score) query.bind(13, m_score);
    if (m_spawn_room) query.bind(14, m_spawn_room);
    query.bind(15, m_species);
    query.bind(16, sql_id);
    if (m_stance != CombatStance::BALANCED) query.bind(17, static_cast<int>(m_stance));
    const std::string tags = StrX::tags_to_string(m_tags);
    if (tags.size()) query.bind(18, tags);
    query.exec();

    // Save any and all buffs/debuffs.
    for (auto b : m_buffs)
        b->save(save_db, sql_id);

    return sql_id;
}

// Checks this Mobile's score.
uint32_t Mobile::score() const { return m_score; }

// Sets a specified buff/debuff on the Actor, or extends an existing buff/debuff.
void Mobile::set_buff(Buff::Type type, uint16_t time, uint32_t power, bool additive_power)
{
    for (auto b : m_buffs)
    {
        if (b->type == type)
        {
            if (time != USHRT_MAX) b->time += time;
            if (additive_power) b->power += power;
            else if (b->power < power) b->power = power;
            return;
        }
    }
    auto new_buff = std::make_shared<Buff>();
    new_buff->type = type;
    new_buff->time = time;
    new_buff->power = power;
    m_buffs.push_back(new_buff);
}

// Sets the gender of this Mobile.
void Mobile::set_gender(Gender gender) { m_gender = gender; }

// Sets the current (and, optionally, maximum) HP of this Mobile.
void Mobile::set_hp(int hp, int hp_max)
{
    m_hp[0] = hp;
    if (hp_max) m_hp[1] = hp_max;
}

// Sets this Mobile's unique ID.
void Mobile::set_id(uint32_t new_id) { m_id = new_id; }

// Sets the location of this Mobile with a Room ID.
void Mobile::set_location(uint32_t room_id)
{
    m_location = room_id;
    if (is_player()) core()->world()->recalc_active_rooms();
}

// As above, but with a string Room ID.
void Mobile::set_location(const std::string &room_id)
{
    if (!room_id.size()) set_location(0);
    else set_location(StrX::hash(room_id));
}

// Adds Mobile metadata.
void Mobile::set_meta(const std::string &key, std::string value)
{
    StrX::find_and_replace(value, " ", "_");
    if (m_metadata.find(key) == m_metadata.end()) m_metadata.insert(std::pair<std::string, std::string>(key, value));
    else m_metadata.at(key) = value;
}

// As above, but with an integer value.
void Mobile::set_meta(const std::string &key, int value) { set_meta(key, std::to_string(value)); }

// As above again, but this time for floats.
void Mobile::set_meta(const std::string &key, float value) { set_meta(key, StrX::ftos(value, 1)); }

// As above, but with an unsigned 32-bit integer.
void Mobile::set_meta_uint(const std::string &key, uint32_t value) { set_meta(key, std::to_string(value)); }

// Sets the name of this Mobile.
void Mobile::set_name(const std::string &name) { m_name = name; }

// Sets this Mobile's spawn room.
void Mobile::set_spawn_room(uint32_t id) { m_spawn_room = id; }

// Sets the species of this Mobile.
void Mobile::set_species(const std::string &species) { m_species = species; }

// Sets this Mobile's combat stance.
void Mobile::set_stance(CombatStance stance) { m_stance = stance; }

// Sets a MobileTag on this Mobile.
void Mobile::set_tag(MobileTag the_tag)
{
    if (m_tags.count(the_tag) > 0) return;
    m_tags.insert(the_tag);
}

// Checks the species of this Mobile.
std::string Mobile::species() const { return m_species; }

// Checks this Mobile's combat stance.
CombatStance Mobile::stance() const { return m_stance; }

// Checks if a MobileTag is set on this Mobile.
bool Mobile::tag(MobileTag the_tag) const { return (m_tags.count(the_tag) > 0); }

// Triggers a single bleed tick.
bool Mobile::tick_bleed(uint32_t power, uint16_t time)
{
    if (!power || tag(MobileTag::ImmunityBleed)) return true;
    const auto room = core()->world()->get_room(m_location);
    const bool fatal = (static_cast<int>(power) >= m_hp[0]);

    room->add_scar(ScarType::BLOOD, SCAR_BLEED_INTENSITY_FROM_BLEED_TICK);
    if (is_player())
    {
        core()->message("{r}You are {R}bleeding {r}rather badly. {w}[{R}-" + std::to_string(power) + "{w}]");
        if (fatal)
        {
            core()->message("{0}{R}You've lost too much blood and collapse, bleeding out on the ground.");
            core()->world()->player()->set_death_reason("died from excessive blood loss");
        }
    }
    else
    {
        const std::shared_ptr<Player> player = core()->world()->player();
        if (player->location() == m_location && room->light() >= Room::LIGHT_VISIBLE)
            core()->message("{r}" + name(NAME_FLAG_CAPITALIZE_FIRST | NAME_FLAG_THE) + " {r}is {R}bleeding {r}rather badly. {w}[{R}-" + std::to_string(power) + "{w}]");
    }
    reduce_hp(power);
    if (!fatal && is_player() && time == 1) core()->message("{r}Your wounds stop bleeding.");
    return !fatal;
}

// Reduce the timer on all buffs.
void Mobile::tick_buffs()
{
    for (size_t i = 0; i < m_buffs.size(); i++)
    {
        if (m_buffs.at(i)->time == USHRT_MAX) continue;

        switch (m_buffs.at(i)->type)
        {
            case Buff::Type::BLEED:
                if (!tick_bleed(m_buffs.at(i)->power, m_buffs.at(i)->time)) return;
                break;
            case Buff::Type::POISON:
                if (!tick_poison(m_buffs.at(i)->power, m_buffs.at(i)->time)) return;
                break;
            default: break;
        }

        if (!--m_buffs.at(i)->time)
            m_buffs.erase(m_buffs.begin() + i--);
    }
}

// Triggers a single poison tick.
bool Mobile::tick_poison(uint32_t power, uint16_t time)
{
    if (!power || tag(MobileTag::ImmunityPoison)) return true;
    const auto room = core()->world()->get_room(m_location);
    const bool fatal = (static_cast<int>(power) >= m_hp[0]);

    if (is_player())
    {
        core()->message("{g}You feel deathly ill from the {G}poison {g}in your veins. {w}[{G}-" + std::to_string(power) + "{w}]");
        if (fatal)
        {
            core()->message("{0}{G}The poison running through your veins is too much, and your body shuts down.");
            core()->world()->player()->set_death_reason("succumbed to poison");
        }
    }
    else
    {
        const std::shared_ptr<Player> player = core()->world()->player();
        if (player->location() == m_location && room->light() >= Room::LIGHT_VISIBLE)
            core()->message("{g}" + name(NAME_FLAG_CAPITALIZE_FIRST | NAME_FLAG_THE) + " {g}takes damage from {G}poison{g}. {w}[{G}-" + std::to_string(power) + "{w}]");
    }
    reduce_hp(power);
    if (!fatal && is_player() && time == 1) core()->message("{g}You feel much better as the poison fades from your system.");
    return !fatal;
}
