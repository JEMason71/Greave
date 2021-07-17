// world/inventory.hpp -- The Inventory class stores a collection of Items, and handles stacking, organizing, saving/loading, etc.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "3rdparty/SQLiteCpp/Database.h"
#include "core/greave.hpp"
#include "world/item.hpp"


class Inventory
{
public:
    enum TagPrefix { NONE = 0, INVENTORY, EQUIPMENT, ROOM, SHOP, MOBILE };

                Inventory(uint8_t tag_prefix);          // Creates a new, blank inventory.
    void        add_item(std::shared_ptr<Item> item, bool force_stack = false); // Adds an Item to this Inventory (this will later handle auto-stacking, etc.)
    void        add_item(const std::string &id, bool force_stack = false);      // As above, but generates a new Item from a template with a specified ID.
    size_t      ammo_pos(std::shared_ptr<Item> item);   // Locates the position of an ammunition item used by the specified weapon.
    void        clear();                                // Erases everything from this inventory.
    size_t      count() const;                          // Returns the number of Items in this Inventory.
    void        erase(size_t pos);                      // Deletes an Item from this Inventory.
    std::shared_ptr<Item> get(size_t pos) const;        // Retrieves an Item from this Inventory.
    std::shared_ptr<Item> get(EquipSlot es) const;      // As above, but retrieves an item based on a given equipment slot.
    void        load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id);   // Loads an Inventory from the save file.
    void        remove_item(size_t pos);                // Removes an Item from this Inventory.
    void        remove_item(EquipSlot es);              // As above, but with a specified equipment slot.
    uint32_t    save(std::shared_ptr<SQLite::Database> save_db);    // Saves this Inventory, returns its SQL ID.
    void        sort();                                 // Sorts the inventory into alphabetical order.

private:
    bool        parser_id_exists(uint16_t id);          // Checks if a given parser ID already exists on an Item in this Inventory.

    std::vector<std::shared_ptr<Item>>  m_items;        // The Items stored in this Inventory.
    uint8_t     m_tag_prefix;                           // The prefix for all tag numbers in this Inventory.
};
