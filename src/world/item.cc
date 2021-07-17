// world/item.cc -- The Item class is for objects that can be picked up and used by the player or other NPCs.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.h"
#include "core/guru.h"
#include "core/mathx.h"
#include "core/random.h"
#include "core/strx.h"
#include "world/item.h"
#include "world/player.h"
#include "world/world.h"


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
const std::string Item::SQL_ITEMS = "CREATE TABLE items ( description TEXT, metadata TEXT, name TEXT NOT NULL, owner_id INTEGER NOT NULL, parser_id INTEGER NOT NULL, rare INTEGER NOT NULL, sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL, stack INTEGER, subtype INTEGER, tags TEXT, type INTEGER, value INTEGER, weight INTEGER NOT NULL )";

const int   Item::APPRAISAL_XP_EASY =               1;      // The amount of appraisal XP gained for an easy item appraisal.
const int   Item::APPRAISAL_XP_HARD =               5;      // The amount of appraisal XP gained for a difficult item appraisal.
const int   Item::APPRAISAL_BASE_SKILL_REQUIRED =   -11;    // The base modifier to appraisal skill required for an item, after taking rarity into account.
const int   Item::APPRAISAL_RARITY_MULTIPLIER =     9;      // The multiplier to the appraisal skill required for an item, per rarity level.
const float Item::WATER_WEIGHT =                    58.68f; // The weight of 1 unit of water.


// Constructor, sets default values.
Item::Item() : m_parser_id(0), m_rarity(1), m_stack(1), m_type(ItemType::NONE), m_type_sub(ItemSub::NONE), m_value(0) { }

// The damage multiplier for ammunition.
float Item::ammo_power() const { return meta_float("ammo_power"); }

// Attempts to guess the value of an item.
int Item::appraised_value()
{
    if (!m_value) return 0;
    else if (meta_int("appraised_value")) return meta_int("appraised_value");

    int required_skill = (m_rarity * APPRAISAL_RARITY_MULTIPLIER) + APPRAISAL_BASE_SKILL_REQUIRED;
    if (required_skill < 0) required_skill = 0;
    const int appraisal_skill = core()->world()->player()->skill_level("APPRAISAL");
    if (appraisal_skill >= required_skill)
    {
        int value_fuzzed = MathX::fuzz(m_value);
        set_meta("appraised_value", value_fuzzed);
        core()->world()->player()->gain_skill_xp("APPRAISAL", APPRAISAL_XP_EASY);
        if (tag(ItemTag::Stackable)) value_fuzzed *= m_stack;
        return value_fuzzed;
    }

    const int diff = required_skill - appraisal_skill;
    int penalty = 0;
    if (diff >= 50) penalty = 1000;
    else if (diff >= 25) penalty = 100;
    else if (diff >= 20) penalty = 50;
    else if (diff >= 10) penalty = 10;
    else penalty = 1;
    const int rolled_penalty = core()->rng()->rnd(penalty);
    int value_appraised;
    if (core()->rng()->rnd(3) == 1) value_appraised = MathX::fuzz(MathX::mixup(m_value / rolled_penalty, true));
    else value_appraised = MathX::fuzz(MathX::mixup(m_value * rolled_penalty, true));
    set_meta("appraised_value", value_appraised);
    core()->world()->player()->gain_skill_xp("APPRAISAL", APPRAISAL_XP_HARD);
    if (tag(ItemTag::Stackable)) value_appraised *= m_stack;
    return value_appraised;
}

// Returns the armour damage reduction value of this Item, if any.
float Item::armour(int bonus_power) const
{
    if ((m_type != ItemType::ARMOUR && m_type != ItemType::SHIELD) || !power()) return 0;
    return std::pow(power() + bonus_power + 4, 1.2) / 100.0f;
}

// Returns thie bleed chance of this Item, if any.
int Item::bleed() const { return meta_int("bleed"); }

// Returns the block modifier% for this Item, if any.
int Item::block_mod() const { return meta_int("block_mod"); }

// Returns this Item's capacity, if any.
int Item::capacity() const { return meta_int("capacity"); }

// Returns this Item's charge, if any.
int Item::charge() const { return meta_int("charge"); }

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
EquipSlot Item::equip_slot() const { return static_cast<EquipSlot>(meta_int("slot")); }

// Checks if this Item is identical to another (except stack size).
bool Item::is_identical(std::shared_ptr<Item> item) const
{
    // We'll go through the checks in order of computationally cheapest first, and leave the more expensive checks to the end.

    // Integer comparison.
    if (m_rarity != item->m_rarity) return false;
    if (m_type != item->m_type) return false;
    if (m_type_sub != item->m_type_sub) return false;
    if (m_value != item->m_value) return false;
    if (m_weight != item->m_weight) return false;

    // String comparison.
    if (m_name != item->m_name) return false;
    if (m_description != item->m_description) return false;

    // Way more complicated comparison stuff below here.

    // For metadata comparison, appraised values might differ. So we'll take that out of the equation.
    auto copy_a = std::make_shared<Item>(*this);
    auto copy_b = std::make_shared<Item>(*item);
    copy_a->clear_meta("appraised_value");
    copy_b->clear_meta("appraised_value");
    if (StrX::metadata_to_string(copy_a->m_metadata) != StrX::metadata_to_string(copy_b->m_metadata)) return false;

    // Tag matches are easier.
    if (StrX::tags_to_string(m_tags) != StrX::tags_to_string(item->m_tags)) return false;

    return true;
}

// Returns the liquid type contained in this Item, if any.
std::string Item::liquid_type() const { return meta("liquid"); }

// Loads a new Item from the save file.
std::shared_ptr<Item> Item::load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id)
{
    auto new_item = std::make_shared<Item>();

    SQLite::Statement query(*save_db, "SELECT * FROM items WHERE sql_id = :id");
    query.bind(":id", sql_id);
    if (query.executeStep())
    {
        ItemType new_type = ItemType::NONE;
        ItemSub new_subtype = ItemSub::NONE;

        if (!query.getColumn("description").isNull()) new_item->set_description(query.getColumn("description").getString());
        if (!query.getColumn("metadata").isNull()) StrX::string_to_metadata(query.getColumn("metadata").getString(), new_item->m_metadata);
        new_item->set_name(query.getColumn("name").getString());
        new_item->m_parser_id = query.getColumn("parser_id").getUInt();
        new_item->m_rarity = query.getColumn("rare").getInt();
        if (!query.isColumnNull("stack")) new_item->m_stack = query.getColumn("stack").getUInt(); else new_item->m_stack = 1;
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
    std::string result = m_metadata.at(key);
    StrX::find_and_replace(result, "_", " ");
    return result;
}

// Retrieves metadata, in float format.
float Item::meta_float(const std::string &key) const
{
    const std::string key_str = meta(key);
    if (!key_str.size()) return 0.0f;
    else return std::stof(key_str);
}

// Retrieves metadata, in int format.
int Item::meta_int(const std::string &key) const
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
    std::string ret = m_name, plural_name = meta("plural_name");
    if (plural && plural_name.size())
    {
        ret = plural_name;
        using_plural_name = true;
    }

    if (m_stack > 1 && !no_count) ret = StrX::number_to_word(m_stack) + " " + name(NAME_FLAG_PLURAL | NAME_FLAG_NO_COUNT);

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
            case ItemType::DRINK:
                full_stats_str += " {c}[{U}" + std::to_string(charge()) + "{c}/{U}" + std::to_string(capacity());
                if (charge()) full_stats_str += " {c}" + liquid_type();
                full_stats_str += "{c}]";
                break;
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
void Item::new_parser_id(uint8_t prefix) { m_parser_id = core()->rng()->rnd(0, 999) + (prefix * 1000); }

// Returns the parry% modifier of this Item, if any.
int Item::parry_mod() const { return meta_int("parry_mod"); }

// Retrieves the current ID of this Item, for parser differentiation.
uint16_t Item::parser_id() const { return m_parser_id; }

// Returns thie poison chance of this Item, if any.
int Item::poison() const { return meta_int("poison"); }

// Retrieves this Item's power.
int Item::power() const { return meta_int("power"); }

// Retrieves this Item's rarity.
int Item::rare() const { return m_rarity; }

// Saves the Item.
void Item::save(std::shared_ptr<SQLite::Database> save_db, uint32_t owner_id)
{
    SQLite::Statement query(*save_db, "INSERT INTO items ( description, metadata, name, owner_id, parser_id, rare, sql_id, stack, subtype, tags, type, value, weight ) VALUES ( :desc, :meta, :name, :owner_id, :parser_id, :rare, :sql_id, :stack, :subtype, :tags, :type, :value, :weight )");
    if (m_description.size()) query.bind(":desc", m_description);
    if (m_metadata.size()) query.bind(":meta", StrX::metadata_to_string(m_metadata));
    query.bind(":name", m_name);
    query.bind(":owner_id", owner_id);
    query.bind(":parser_id", m_parser_id);
    query.bind(":rare", m_rarity);
    query.bind(":sql_id", core()->sql_unique_id());
    if (m_stack != 1) query.bind(":stack", m_stack);
    if (m_type_sub != ItemSub::NONE) query.bind(":subtype", static_cast<int>(m_type_sub));
    if (m_tags.size()) query.bind(":tags", StrX::tags_to_string(m_tags));
    if (m_type != ItemType::NONE) query.bind(":type", static_cast<int>(m_type));
    if (m_value) query.bind(":value", m_value);
    query.bind(":weight", m_weight);
    query.exec();
}

// Sets the charge level of this Item.
void Item::set_charge(int new_charge) { set_meta("charge", new_charge); }

// Sets this Item's description.
void Item::set_description(const std::string &desc) { m_description = desc; }

// Sets this Item's equipment slot.
void Item::set_equip_slot(EquipSlot es) { set_meta("slot", static_cast<int>(es)); }

// Sets the liquid contents of this Item.
void Item::set_liquid(const std::string &new_liquid) { set_meta("liquid", new_liquid); }

// Adds Item metadata.
void Item::set_meta(const std::string &key, std::string value)
{
    if (!value.size())
    {
        clear_meta(key);
        return;
    }
    StrX::find_and_replace(value, " ", "_");
    if (m_metadata.find(key) == m_metadata.end()) m_metadata.insert(std::pair<std::string, std::string>(key, value));
    else m_metadata.at(key) = value;
}

// As above, but with an integer value.
void Item::set_meta(const std::string &key, int value)
{
    if (!value) clear_meta(key);
    else set_meta(key, std::to_string(value));
}

// As above, but with an unsigned integer value.
void Item::set_meta(const std::string &key, uint32_t value)
{
    if (!value) clear_meta(key);
    else set_meta(key, std::to_string(value));
}

// As above again, but this time for floats.
void Item::set_meta(const std::string &key, float value)
{
    if (!value) clear_meta(key);
    else set_meta(key, StrX::ftos(value, 1));
}

// Sets the name of this Item.
void Item::set_name(const std::string &name) { m_name = name; }

// Sets this item's parser ID prefix.
void Item::set_parser_id_prefix(uint8_t prefix)
{
    if (!m_parser_id)
    {
        new_parser_id(prefix);
        return;
    }
    int old_prefix = m_parser_id / 1000;
    m_parser_id -= (old_prefix * 1000);
    m_parser_id += (prefix * 1000);
}

// Sets this Item's rarity.
void Item::set_rare(int rarity) { m_rarity = rarity; }

// Sets the stack size for this Item.
void Item::set_stack(uint32_t size) { m_stack = size; }

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
float Item::speed() const { return meta_float("speed"); }

// Splits an Item into a stack.
std::shared_ptr<Item> Item::split(int split_count)
{
    const bool stackable = tag(ItemTag::Stackable);
    if (split_count < 0) throw std::runtime_error("Invalid item stack split: " + m_name);
    if (!split_count || (split_count == 1 && !stackable) || static_cast<int64_t>(split_count) == m_stack) return nullptr;
    if (!stackable) throw std::runtime_error("Attempt to split unstackable item: " + m_name);
    if (static_cast<unsigned int>(split_count) > m_stack) throw std::runtime_error("Invalid stack split size: " + m_name);
    auto new_item = std::make_shared<Item>(*this);
    new_item->m_stack = split_count;
    m_stack -= split_count;
    return new_item;
}

// Retrieves the stack size of this Item.
uint32_t Item::stack() const { return m_stack; }

// Like name(), but provides an appropriate name for a given stack size. Works on non-stackable items too.
std::string Item::stack_name(int stack_size, int flags)
{
    if (!tag(ItemTag::Stackable) || stack_size == -1 || stack_size == static_cast<int>(m_stack)) return name(flags);
    if (stack_size == 1) return name(NAME_FLAG_NO_COUNT | flags);

    std::string the_str;
    if ((flags & NAME_FLAG_THE) == NAME_FLAG_THE)
    {
        if (!tag(ItemTag::ProperNoun))
        {
            the_str = "the ";
            if ((flags & NAME_FLAG_CAPITALIZE_FIRST) == NAME_FLAG_CAPITALIZE_FIRST)
            {
                the_str = "The ";
                flags ^= NAME_FLAG_CAPITALIZE_FIRST;
            }
        }
        flags ^= NAME_FLAG_THE;
    }

    return the_str + StrX::number_to_word(stack_size) + " " + name(NAME_FLAG_PLURAL | NAME_FLAG_NO_COUNT | flags);
}

// Returns the ItemSub (sub-type) of this Item.
ItemSub Item::subtype() const { return m_type_sub; }

// Checks if a tag is set on this Item.
bool Item::tag(ItemTag the_tag) const { return (m_tags.count(the_tag) > 0); }

// Returns the ItemType of this Item.
ItemType Item::type() const { return m_type; }

// The Item's value in money.
uint32_t Item::value(bool individual) const
{
    if (individual || !tag(ItemTag::Stackable)) return m_value;
    else return m_value * m_stack;
}

// The Item's warmth rating, if any.
int Item::warmth() const { return meta_int("warmth"); }

// The Item's weight, in pacs.
uint32_t Item::weight(bool individual) const
{
    uint32_t water_weight = 0;
    if (m_type == ItemType::DRINK) water_weight = std::round(charge() * WATER_WEIGHT);
    if (individual || !tag(ItemTag::Stackable)) return m_weight + water_weight;
    else return (m_weight + water_weight) * m_stack;
}
