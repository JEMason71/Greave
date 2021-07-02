// world/item.cpp -- The Item class is for objects that can be picked up and used by the player or other NPCs.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.hpp"
#include "core/random.hpp"
#include "core/strx.hpp"
#include "world/item.hpp"


// The SQL table construction string for saving items.
const std::string Item::SQL_ITEMS = "CREATE TABLE items ( description TEXT, equip_slot INTEGER, metadata TEXT, name TEXT NOT NULL, owner_id INTEGER NOT NULL, "
    "parser_id INTEGER NOT NULL, power INTEGER, speed REAL, sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL, subtype INTEGER, tags TEXT, type INTEGER )";


// Constructor, sets default values.
Item::Item() : m_equip_slot(EquipSlot::NONE), m_parser_id(0), m_power(0), m_type(ItemType::NONE), m_type_sub(ItemSub::NONE) { }

// Clears a metatag from an Item. Use with caution!
void Item::clear_meta(const std::string &key) { m_metadata.erase(key); }

// Clears a tag on this Item.
void Item::clear_tag(ItemTag the_tag)
{
    if (!(m_tags.count(the_tag) > 0)) return;
    m_tags.erase(the_tag);
}

// Retrieves this Item's description.
std::string Item::desc() const { return m_description; }

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
        if (!query.getColumn("power").isNull()) new_item->m_power = query.getColumn("power").getInt();
        if (!query.getColumn("speed").isNull()) new_item->m_speed = query.getColumn("speed").getDouble();
        if (!query.isColumnNull("subtype")) new_subtype = static_cast<ItemSub>(query.getColumn("subtype").getInt());
        if (!query.getColumn("tags").isNull()) StrX::string_to_tags(query.getColumn("tags").getString(), new_item->m_tags);
        if (!query.isColumnNull("type")) new_type = static_cast<ItemType>(query.getColumn("type").getInt());

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

// Accesses the metadata map directly. Use with caution!
std::map<std::string, std::string>* Item::meta_raw() { return &m_metadata; }

// Retrieves the name of thie Item.
std::string Item::name(ItemName level) const
{
    if (level == ItemName::BASIC) return m_name;
    std::string name = m_name + " ";
    std::string inv_stats, room_stats;

    switch (m_type)
    {
        case ItemType::LIGHT: room_stats += "{Y}<gl{W}o{Y}wing> "; break;
        case ItemType::WEAPON: inv_stats += "{c}<{U}" + std::to_string(power()) + "{c}/{U}" + StrX::ftos(speed(), true) + "{c}> "; break;
        default: break;
    }
    inv_stats += "{B}{" + StrX::itos(m_parser_id, 4) + "} ";

    if (level == ItemName::INVENTORY && inv_stats.size()) name += inv_stats;
    name += room_stats;
    name.pop_back();

    return name;
}

// Generates a new parser ID for this Item.
void Item::new_parser_id() { m_parser_id = core()->rng()->rnd(1, 9999); }

// Retrieves the current ID of this Item, for parser differentiation.
uint16_t Item::parser_id() const { return m_parser_id; }

// Retrieves this Item's power.
uint16_t Item::power() const { return m_power; }

// Saves the Item.
void Item::save(std::shared_ptr<SQLite::Database> save_db, uint32_t owner_id)
{
    SQLite::Statement query(*save_db, "INSERT INTO items ( description, equip_slot, metadata, name, owner_id, parser_id, power, speed, sql_id, subtype, tags, type ) "
        "VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )");
    if (m_description.size()) query.bind(1, m_description);
    if (m_equip_slot != EquipSlot::NONE) query.bind(2, static_cast<int>(m_equip_slot));
    if (m_metadata.size()) query.bind(3, StrX::metadata_to_string(m_metadata));
    query.bind(4, m_name);
    query.bind(5, owner_id);
    query.bind(6, m_parser_id);
    if (m_power) query.bind(7, m_power);
    if (m_speed) query.bind(8, m_speed);
    query.bind(9, core()->sql_unique_id());
    if (m_type_sub != ItemSub::NONE) query.bind(10, static_cast<int>(m_type_sub));
    if (m_tags.size()) query.bind(11, StrX::tags_to_string(m_tags));
    if (m_type != ItemType::NONE) query.bind(12, static_cast<int>(m_type));
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

// Sets the name of this Item.
void Item::set_name(const std::string &name) { m_name = name; }

// Sets the power of this Item.
void Item::set_power(uint16_t power) { m_power = power; }

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

// Retrieves the speed of this Item.
float Item::speed() const { return m_speed; }

// Returns the ItemSub (sub-type) of this Item.
ItemSub Item::subtype() const { return m_type_sub; }

// Checks if a tag is set on this Item.
bool Item::tag(ItemTag the_tag) const { return (m_tags.count(the_tag) > 0); }

// Returns the ItemType of this Item.
ItemType Item::type() const { return m_type; }
