// world/inventory.h -- The Inventory class stores a collection of Items, and handles stacking, organizing, saving/loading, etc.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_WORLD_INVENTORY_H_
#define GREAVE_WORLD_INVENTORY_H_

#include "3rdparty/SQLiteCpp/Database.h"
#include "world/item.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>


class Inventory
{
public:
    static constexpr int    PID_PREFIX_INVENTORY =  1;  // All items in an inventory have a parser ID prefixed with 1 (e.g. 1234).
    static constexpr int    PID_PREFIX_EQUIPMENT =  2;  // All items in an equipment inventory have a parser ID prefixed with 2 (e.g. 2345).
    static constexpr int    PID_PREFIX_ROOM =       3;  // All items on the ground in a room have a parser ID prefixed with 3 (e.g. 3456).
    static constexpr int    PID_PREFIX_SHOP =       4;  // All items for sale in a shop have a parser ID prefixed with 4 (e.g. 4567).
    static constexpr int    PID_PREFIX_MOBILE =     9;  // All mobiles have a parser ID prefixed with 9 (e.g. 9876).

                Inventory(uint8_t pid_prefix);          // Creates a new, blank inventory.
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

    std::vector<std::shared_ptr<Item>>  items_;         // The Items stored in this Inventory.
    uint8_t                             pid_prefix_;    // The prefix for all parser ID numbers in this Inventory.
};

#endif  // GREAVE_WORLD_INVENTORY_H_
