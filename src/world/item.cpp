// world/item.cpp -- The Item class is for objects that can be picked up and used by the player or other NPCs.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.hpp"
#include "core/guru.hpp"
#include "core/random.hpp"
#include "core/strx.hpp"
#include "world/item.hpp"


const int Item::NAME_FLAG_A =                   1;      // Much like NAME_FLAG_THE, but using 'a' or 'an' instead of 'the'.
const int Item::NAME_FLAG_CAPITALIZE_FIRST =    2;      // Capitalize the first letter of the Item's name (including the "The") if set.
const int Item::NAME_FLAG_CORE_STATS =          4;      // Displays core stats on Item names, such as if an Item is glowing.
const int Item::NAME_FLAG_ID =                  8;      // Displays an item's ID number, such as {1234}.
const int Item::NAME_FLAG_FULL_STATS =          16;     // Displays some basic stats next to an item's name.
const int Item::NAME_FLAG_NO_COLOUR =           32;     // Strips colour out of an Item's name.
const int Item::NAME_FLAG_NO_COUNT =            64;     // Ignore the stack size on this item.
const int Item::NAME_FLAG_PLURAL =              128;    // Return a plural of the Item's name (e.g. apple -> apples).
const int Item::NAME_FLAG_RARE =                256;    // Include the Item's rarity value in its name.
const int Item::NAME_FLAG_THE =                 512;    // Precede the Item's name with 'the', unless the name is a proper noun.

// The SQL table construction string for saving items.
const std::string Item::SQL_ITEMS = "CREATE TABLE items ( description TEXT, equip_slot INTEGER, metadata TEXT, name TEXT NOT NULL, owner_id INTEGER NOT NULL, "
    "parser_id INTEGER NOT NULL, plural_name TEXT, power INTEGER, rare INTEGER NOT NULL, speed REAL, sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL, subtype INTEGER, tags TEXT, "
    "type INTEGER, value INTEGER, weight INTEGER NOT NULL )";


// Constructor, sets default values.
Item::Item() : m_equip_slot(EquipSlot::NONE), m_parser_id(0), m_power(0), m_rarity(1), m_speed(0), m_type(ItemType::NONE), m_type_sub(ItemSub::NONE), m_value(0) { }

// Returns the armour damage reduction value of this Item, if any.
float Item::armour(int bonus_power) const
{
    if ((m_type != ItemType::ARMOUR && m_type != ItemType::SHIELD) || !m_power) return 0;
    return std::pow(m_power + bonus_power + 4, 1.2) / 100.0f;
}

// Returns the block modifier% for this Item, if any.
int Item::block_mod() const { return meta_int("block_mod"); }

// Clears a metatag from an Item. Use with caution!
void Item::clear_meta(const std::string &key) { m_metadata.erase(key); }

// Clears a tag on this Item.
void Item::clear_tag(ItemTag the_tag)
{
    if (!(m_tags.count(the_tag) > 0)) return;
    m_tags.erase(the_tag);
}

// Retrieves this Item's critical power, if any.
int Item::crit() const { return meta_int("crit"); }

// Retrieves this Item's damage type, if any.
DamageType Item::damage_type() const { return static_cast<DamageType>(meta_int("damage_type")); }

// Returns a string indicator of this Item's damage type (e.g. edged = E)
std::string Item::damage_type_string() const
{
    switch (damage_type())
    {
        case DamageType::ACID: return "Ac";
        case DamageType::BALLISTIC: return "B";
        case DamageType::CRUSHING: return "C";
        case DamageType::EDGED: return "E";
        case DamageType::ENERGY: return "En";
        case DamageType::EXPLOSIVE: return "Ex";
        case DamageType::KINETIC: return "K";
        case DamageType::PIERCING: return "P";
        case DamageType::PLASMA: return "Pm";
        case DamageType::POISON: return "Ps";
        case DamageType::RENDING: return "R";
        default:
            core()->guru()->nonfatal("Unable to determine item damage type: " + name(), Guru::ERROR);
            return "";
    }
}

// Retrieves this Item's description.
std::string Item::desc() const { return m_description; }

// Returns the dodge modifier% for this Item, if any.
int Item::dodge_mod() const { return meta_int("dodge_mod"); }

// Checks what slot this Item equips in, if any.
EquipSlot Item::equip_slot() const { return m_equip_slot; }

// Loads a new Item from the save file.
std::shared_ptr<Item> Item::load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id)
{
    auto new_item = std::make_shared<Item>();

    SQLite::Statement query(*save_db, "SELECT * FROM items WHERE sql_id = ?");
    query.bind(1, sql_id);
    if (query.executeStep())
    {
        ItemType new_type = ItemType::NONE;
        ItemSub new_subtype = ItemSub::NONE;

        if (!query.getColumn("description").isNull()) new_item->set_description(query.getColumn("description").getString());
        if (!query.getColumn("equip_slot").isNull()) new_item->set_equip_slot(static_cast<EquipSlot>(query.getColumn("equip_slot").getInt()));
        if (!query.getColumn("metadata").isNull()) StrX::string_to_metadata(query.getColumn("metadata").getString(), new_item->m_metadata);
        new_item->set_name(query.getColumn("name").getString());
        new_item->m_parser_id = query.getColumn("parser_id").getUInt();
        if (!query.getColumn("plural_name").isNull()) new_item->m_plural_name = query.getColumn("plural_name").getString();
        if (!query.getColumn("power").isNull()) new_item->m_power = query.getColumn("power").getInt();
        new_item->m_rarity = query.getColumn("rare").getInt();
        if (!query.getColumn("speed").isNull()) new_item->m_speed = query.getColumn("speed").getDouble();
        if (!query.isColumnNull("subtype")) new_subtype = static_cast<ItemSub>(query.getColumn("subtype").getInt());
        if (!query.getColumn("tags").isNull()) StrX::string_to_tags(query.getColumn("tags").getString(), new_item->m_tags);
        if (!query.isColumnNull("type")) new_type = static_cast<ItemType>(query.getColumn("type").getInt());
        if (!query.isColumnNull("value")) new_item->m_value = query.getColumn("value").getUInt();
        new_item->m_weight = query.getColumn("weight").getUInt();

        new_item->set_type(new_type, new_subtype);
    }
    else throw std::runtime_error("Could not retrieve data for item ID " + std::to_string(sql_id));

    return new_item;
}

// Retrieves Item metadata.
std::string Item::meta(const std::string &key) const
{
    if (m_metadata.find(key) == m_metadata.end()) return "";
    else return m_metadata.at(key);
}

// Retrieves metadata, in int format.
int Item::meta_int(std::string key) const
{
    const std::string key_str = meta(key);
    if (!key_str.size()) return 0;
    else return std::stoi(key_str);
}

// Accesses the metadata map directly. Use with caution!
std::map<std::string, std::string>* Item::meta_raw() { return &m_metadata; }

// Retrieves the name of thie Item.
std::string Item::name(int flags) const
{
    const bool no_count = ((flags & Item::NAME_FLAG_NO_COUNT) == Item::NAME_FLAG_NO_COUNT);
    const bool a = (((flags & Item::NAME_FLAG_A) == Item::NAME_FLAG_A) && no_count); //(m_stack == 1 || no_count));
    const bool capitalize_first = ((flags & Item::NAME_FLAG_CAPITALIZE_FIRST) == Item::NAME_FLAG_CAPITALIZE_FIRST);
    const bool no_colour = ((flags & Item::NAME_FLAG_NO_COLOUR) == Item::NAME_FLAG_NO_COLOUR);
    const bool full_stats = ((flags & Item::NAME_FLAG_FULL_STATS) == Item::NAME_FLAG_FULL_STATS);
    const bool core_stats = full_stats || ((flags & Item::NAME_FLAG_CORE_STATS) == Item::NAME_FLAG_CORE_STATS);
    const bool id = full_stats || ((flags & Item::NAME_FLAG_ID) == Item::NAME_FLAG_ID);
    const bool plural = (((flags & Item::NAME_FLAG_PLURAL) == Item::NAME_FLAG_PLURAL)); //|| (m_stack > 1 && !no_count));
    const bool the = ((flags & Item::NAME_FLAG_THE) == Item::NAME_FLAG_THE);
    const bool rarity = ((flags & Item::NAME_FLAG_RARE) == Item::NAME_FLAG_RARE);

    bool using_plural_name = false;
    std::string ret = m_name;
    if (plural && m_plural_name.size())
    {
        ret = m_plural_name;
        using_plural_name = true;
    }

    //if (m_stack > 1 && !no_count) ret = StrX::number_to_word(m_stack) + " " + name(NAME_FLAG_PLURAL | NAME_FLAG_NO_COUNT);

    if (the && !tag(ItemTag::ProperNoun)) ret = "the " + ret;
    else if (a && !tag(ItemTag::PluralName) && !tag(ItemTag::NoA) && !tag(ItemTag::ProperNoun))
    {
        if (StrX::is_vowel(ret[0])) ret = "an " + ret;
        else ret = "a " + ret;
    }
    if (capitalize_first && ret[0] >= 'a' && ret[0] <= 'z') ret[0] -= 32;
    if (plural && !using_plural_name && ret.back() != 's') ret += "s";

    if (core_stats || full_stats)
    {
        std::string core_stats_str, full_stats_str;
        switch (m_type)
        {
            case ItemType::ARMOUR: case ItemType::SHIELD: full_stats_str += " {c}[{U}" + std::to_string(power()) + "{c}]"; break;
            case ItemType::LIGHT: core_stats_str += " {Y}<gl{W}o{Y}wing>"; break;
            case ItemType::WEAPON: full_stats_str += " {c}<{U}" + std::to_string(power()) + "{c}" + damage_type_string() + "/{U}" + StrX::ftos(speed(), true) + "{c}>"; break;
            default: break;
        }
        if (core_stats && core_stats_str.size()) ret += core_stats_str;
        if (full_stats && full_stats_str.size()) ret += full_stats_str;
    }
    if (rarity)
    {
        std::string colour_a = "{w}", colour_b = "{w}";
        switch (m_rarity)
        {
            case 4: case 5: case 6: colour_a = "{U}"; colour_b = "{C}"; break;
            case 7: case 8: colour_a = "{g}"; colour_b = "{G}"; break;
            case 9: colour_a = "{m}"; colour_b = "{M}"; break;
            case 10: colour_a = "{y}"; colour_b = "{Y}"; break;
            case 11: colour_a = "{r}"; colour_b = "{R}"; break;
        }
        if (m_rarity == 12) ret += " {M}[" + StrX::rainbow_text("RARE-12", "mB") + "{M}]";
        else ret += " " + colour_a + "[" + colour_b + "RARE-" + std::to_string(m_rarity) + colour_a + "]";
    }
    if (id) ret += " {B}{" + StrX::itos(m_parser_id, 4) + "}";
    if (no_colour) ret = StrX::strip_ansi(ret);
    return ret;
}

// Generates a new parser ID for this Item.
void Item::new_parser_id() { m_parser_id = core()->rng()->rnd(1, 9999); }

// Returns the parry% modifier of this Item, if any.
int Item::parry_mod() const { return meta_int("parry_mod"); }

// Retrieves the current ID of this Item, for parser differentiation.
uint16_t Item::parser_id() const { return m_parser_id; }

// Retrieves this Item's power.
uint16_t Item::power() const { return m_power; }

// Retrieves this Item's rarity.
uint8_t Item::rare() const { return m_rarity; }

// Saves the Item.
void Item::save(std::shared_ptr<SQLite::Database> save_db, uint32_t owner_id)
{
    SQLite::Statement query(*save_db, "INSERT INTO items ( description, equip_slot, metadata, name, owner_id, parser_id, plural_name, power, rare, speed, sql_id, subtype, tags, "
        "type, value, weight ) VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )");
    if (m_description.size()) query.bind(1, m_description);
    if (m_equip_slot != EquipSlot::NONE) query.bind(2, static_cast<int>(m_equip_slot));
    if (m_metadata.size()) query.bind(3, StrX::metadata_to_string(m_metadata));
    query.bind(4, m_name);
    query.bind(5, owner_id);
    query.bind(6, m_parser_id);
    if (m_plural_name.size()) query.bind(7, m_plural_name);
    if (m_power) query.bind(8, m_power);
    query.bind(9, m_rarity);
    if (m_speed) query.bind(10, m_speed);
    query.bind(11, core()->sql_unique_id());
    if (m_type_sub != ItemSub::NONE) query.bind(12, static_cast<int>(m_type_sub));
    if (m_tags.size()) query.bind(13, StrX::tags_to_string(m_tags));
    if (m_type != ItemType::NONE) query.bind(14, static_cast<int>(m_type));
    if (m_value) query.bind(15, m_value);
    query.bind(16, m_weight);
    query.exec();
}

// Sets this Item's description.
void Item::set_description(const std::string &desc) { m_description = desc; }

// Sets this Item's equipment slot.
void Item::set_equip_slot(EquipSlot es) { m_equip_slot = es; }

// Adds Item metadata.
void Item::set_meta(const std::string &key, const std::string &value)
{
    if (m_metadata.find(key) == m_metadata.end()) m_metadata.insert(std::pair<std::string, std::string>(key, value));
    else m_metadata.at(key) = value;
}

// As above, but with an integer value.
void Item::set_meta(const std::string &key, int value) { set_meta(key, std::to_string(value)); }

// Sets the name of this Item.
void Item::set_name(const std::string &name, const std::string &plural_name)
{
    m_name = name;
    m_plural_name = plural_name;
}

// Sets the power of this Item.
void Item::set_power(uint16_t power) { m_power = power; }

// Sets this Item's rarity.
void Item::set_rare(uint8_t rarity) { m_rarity = rarity; }

// Sets the speed of this Item.
void Item::set_speed(float speed) { m_speed = speed; }

// Sets a tag on this Item.
void Item::set_tag(ItemTag the_tag)
{
    if (m_tags.count(the_tag) > 0) return;
    m_tags.insert(the_tag);
}

// Sets the type of this Item.
void Item::set_type(ItemType type, ItemSub sub)
{
    m_type = type;
    m_type_sub = sub;
}

// Sets this Item's value.
void Item::set_value(uint32_t val) { m_value = val; }

// Sets this Item's weight.
void Item::set_weight(uint32_t pacs) { m_weight = pacs; }

// Retrieves the speed of this Item.
float Item::speed() const { return m_speed; }

// Returns the ItemSub (sub-type) of this Item.
ItemSub Item::subtype() const { return m_type_sub; }

// Checks if a tag is set on this Item.
bool Item::tag(ItemTag the_tag) const { return (m_tags.count(the_tag) > 0); }

// Returns the ItemType of this Item.
ItemType Item::type() const { return m_type; }

// The Item's value in money.
uint32_t Item::value() const { return m_value; }

// The Item's weight, in pacs.
uint32_t Item::weight() const { return m_weight; }
