// world/inventory.cpp -- The Inventory class stores a collection of Items, and handles stacking, organizing, saving/loading, etc.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/world.hpp"


#include "core/guru.hpp"
// Adds an Item to this Inventory (this will later handle auto-stacking, etc.)
void Inventory::add_item(std::shared_ptr<Item> item)
{
    // Checks if there's anything else here that can be stacked.
    if (item->tag(ItemTag::Stackable))
    {
        for (size_t i = 0; i < m_items.size(); i++)
        {
            if (!m_items.at(i)->tag(ItemTag::Stackable)) continue;
            if (item->is_identical(m_items.at(i)))
            {
                m_items.at(i)->set_stack(item->stack() + m_items.at(i)->stack());
                return;
            }
        }
    }

    // Check the Item's hex ID. If it's unset, or if another Item in the Inventory shares its ID, we'll need a new one.
    // Infinite loops relying on RNG to break out are VERY BAD so let's put a threshold on this bad boy.
    int tries = 0;
    while ((!item->parser_id() || parser_id_exists(item->parser_id())) && ++tries < 10000)
        item->new_parser_id();
    m_items.push_back(item);
}

// As above, but generates a new Item from a template with a specified ID.
void Inventory::add_item(const std::string &id) { add_item(core()->world()->get_item(id)); }

// Locates the position of an ammunition item used by the specified weapon.
size_t Inventory::ammo_pos(std::shared_ptr<Item> item)
{
    if (item->subtype() != ItemSub::RANGED || item->tag(ItemTag::NoAmmo)) return SIZE_MAX;
    ItemSub ammo_type = ItemSub::NONE;
    if (item->tag(ItemTag::AmmoArrow)) ammo_type = ItemSub::ARROW;
    else if (item->tag(ItemTag::AmmoBolt)) ammo_type = ItemSub::BOLT;
    else throw std::runtime_error("Could not determine ammo type for " + item->name());
    for (size_t i = 0; i < m_items.size(); i++)
    {
        const auto inv_item = m_items.at(i);
        if (inv_item->type() == ItemType::AMMO && inv_item->subtype() == ammo_type) return i;
    }
    return SIZE_MAX;
}

// Returns the number of Items in this Inventory.
size_t Inventory::count() const { return m_items.size(); }

// Deletes an Item from this Inventory.
void Inventory::erase(size_t pos)
{
    if (pos >= m_items.size()) throw std::runtime_error("Invalid inventory position requested.");
    m_items.erase(m_items.begin() + pos);
}

// Retrieves an Item from this Inventory.
std::shared_ptr<Item> Inventory::get(size_t pos) const
{
    if (pos >= m_items.size()) throw std::runtime_error("Invalid inventory position requested.");
    return m_items.at(pos);
}

// As above, but retrieves an item based on a given equipment slot.
std::shared_ptr<Item> Inventory::get(EquipSlot es) const
{
    for (auto item : m_items)
        if (item->equip_slot() == es) return item;
    return nullptr;
}

// Loads an Inventory from the save file.
void Inventory::load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id)
{
    m_items.clear();
    SQLite::Statement query(*save_db, "SELECT sql_id FROM items WHERE owner_id = ? ORDER BY sql_id ASC");
    query.bind(1, sql_id);
    bool loaded_items = false;
    while (query.executeStep())
    {
        auto new_item = Item::load(save_db, query.getColumn("sql_id").getUInt());
        m_items.push_back(new_item);
        loaded_items = true;
    }
    if (!loaded_items) throw std::runtime_error("Could not load inventory data " + std::to_string(sql_id));
}

// Checks if a given parser ID already exists on an Item in this Inventory.
bool Inventory::parser_id_exists(uint16_t id)
{
    for (auto item : m_items)
        if (item->parser_id() == id) return true;
    return false;
}

// Removes an Item from this Inventory.
void Inventory::remove_item(size_t pos)
{
    if (pos >= m_items.size()) throw std::runtime_error("Attempt to remove item with invalid inventory position.");
    m_items.erase(m_items.begin() + pos);
}

// As above, but with a specified equipment slot.
void Inventory::remove_item(EquipSlot es)
{
    for (size_t i = 0; i < m_items.size(); i++)
    {
        if (m_items.at(i)->equip_slot() == es)
        {
            remove_item(i);
            return;
        }
    }
    core()->guru()->nonfatal("Attempt to remove empty equipment slot item.", Guru::ERROR);
}

// Saves this Inventory, returns its SQL ID.
uint32_t Inventory::save(std::shared_ptr<SQLite::Database> save_db)
{
    if (!m_items.size()) return 0;
    const uint32_t sql_id = core()->sql_unique_id();
    for (size_t i = 0; i < m_items.size(); i++)
        m_items.at(i)->save(save_db, sql_id);
    return sql_id;
}
