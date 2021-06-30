// world/item.cpp -- The Item class is for objects that can be picked up and used by the player or other NPCs.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.hpp"
#include "core/random.hpp"
#include "core/strx.hpp"
#include "world/item.hpp"


// The SQL table construction string for saving items.
const std::string Item::SQL_ITEMS = "CREATE TABLE items ( equip_slot INTEGER, hex_id INTEGER NOT NULL, metadata TEXT, name TEXT NOT NULL, owner_id INTEGER NOT NULL, "
    "power INTEGER, sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL, subtype INTEGER, tags TEXT, type INTEGER )";


// Constructor, sets default values.
Item::Item() : m_equip_slot(EquipSlot::NONE), m_hex_id(0), m_power(0), m_type(ItemType::NONE), m_type_sub(ItemSub::NONE) { }

// Clears a metatag from an Item. Use with caution!
void Item::clear_meta(const std::string &key) { m_metadata.erase(key); }

// Clears a tag on this Item.
void Item::clear_tag(ItemTag the_tag)
{
    if (!(m_tags.count(the_tag) > 0)) return;
    m_tags.erase(the_tag);
}

// Checks what slot this Item equips in, if any.
EquipSlot Item::equip_slot() const { return m_equip_slot; }

// Retrieves the current hex ID of this Item.
uint16_t Item::hex_id() const { return m_hex_id; }

// Loads a new Item from the save file.
std::shared_ptr<Item> Item::load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id)
{
    auto new_item = std::make_shared<Item>();

    SQLite::Statement query(*save_db, "SELECT * FROM items WHERE sql_id = ?");
    query.bind(1, sql_id);
    if (query.executeStep())
    {
        new_item->set_name(query.getColumn("name").getString());
        ItemType new_type = ItemType::NONE;
        ItemSub new_subtype = ItemSub::NONE;
        if (!query.isColumnNull("type")) new_type = static_cast<ItemType>(query.getColumn("type").getInt());
        if (!query.isColumnNull("subtype")) new_subtype = static_cast<ItemSub>(query.getColumn("subtype").getInt());
        new_item->set_type(new_type, new_subtype);
        if (!query.getColumn("tags").isNull()) StrX::string_to_tags(query.getColumn("tags").getString(), new_item->m_tags);
        if (!query.getColumn("metadata").isNull()) StrX::string_to_metadata(query.getColumn("metadata").getString(), new_item->m_metadata);
        new_item->m_hex_id = query.getColumn("hex_id").getUInt();
        if (!query.getColumn("equip_slot").isNull()) new_item->set_equip_slot(static_cast<EquipSlot>(query.getColumn("equip_slot").getInt()));
        if (!query.getColumn("power").isNull()) new_item->set_power(query.getColumn("power").getInt());
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
    std::string name = m_name;
    if (m_type == ItemType::LIGHT) name += " {Y}<gl{W}o{Y}wing>";
    if (level == ItemName::INVENTORY) name += " {B}{" + StrX::itoh(m_hex_id, 3) + "}";
    return name;
}

// Generates a new hex ID for this Item.
void Item::new_hex_id() { m_hex_id = core()->rng()->rnd(1, 0xFFF); }

// Retrieves this Item's power.
uint16_t Item::power() const { return m_power; }

// Saves the Item.
void Item::save(std::shared_ptr<SQLite::Database> save_db, uint32_t owner_id)
{
    SQLite::Statement query(*save_db, "INSERT INTO items ( sql_id, owner_id, name, type, subtype, tags, metadata, hex_id, equip_slot, power ) "
        "VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )");
    query.bind(1, core()->sql_unique_id());
    query.bind(2, owner_id);
    query.bind(3, m_name);
    if (m_type != ItemType::NONE) query.bind(4, static_cast<int>(m_type));
    if (m_type_sub != ItemSub::NONE) query.bind(5, static_cast<int>(m_type_sub));
    if (m_tags.size()) query.bind(6, StrX::tags_to_string(m_tags));
    if (m_metadata.size()) query.bind(7, StrX::metadata_to_string(m_metadata));
    query.bind(8, m_hex_id);
    if (m_equip_slot != EquipSlot::NONE) query.bind(9, static_cast<int>(m_equip_slot));
    if (m_power) query.bind(10, m_power);
    query.exec();
}

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

// Returns the ItemSub (sub-type) of this Item.
ItemSub Item::subtype() const { return m_type_sub; }

// Checks if a tag is set on this Item.
bool Item::tag(ItemTag the_tag) const { return (m_tags.count(the_tag) > 0); }

// Returns the ItemType of this Item.
ItemType Item::type() const { return m_type; }
