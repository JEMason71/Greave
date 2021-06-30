// world/item.hpp -- The Item class is for objects that can be picked up and used by the player or other NPCs.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

namespace SQLite { class Database; }    // defined in 3rdparty/SQLiteCpp/Database.h

// The slot that an item is equipped in.
enum class EquipSlot : uint8_t { NONE, HAND_MAIN, HAND_OFF, BODY, ARMOUR, ABOUT_BODY, HEAD, HANDS, FEET };

// ItemType is the primary type of Item (e.g. weapon, food, etc.)
enum class ItemType : uint16_t { NONE, KEY };

// ItemSub is for sub-types of items, e.g. a tool could sub-classify itself here.
enum class ItemSub : uint16_t { NONE };

enum class ItemTag : uint16_t {
    // Unlike RoomTags, there's no over/under 10,000 special rule for ItemTags. Items are saved in their entirety.
};

class Item
{
public:
    static const std::string    SQL_ITEMS;  // The SQL table construction string for saving items.

                Item();                             // Constructor, sets default values.
    void        clear_meta(const std::string &key); // Clears a metatag from an Item. Use with caution!
    void        clear_tag(ItemTag the_tag);         // Clears a tag on this Item.
    EquipSlot   equip_slot() const;                 // Checks what slot this Item equips in, if any.
    uint16_t    hex_id() const;                     // Retrieves the current hex ID of this Item.
    static std::shared_ptr<Item> load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id);  // Loads a new Item from the save file.
    std::string meta(const std::string &key) const; // Retrieves Item metadata.
    std::map<std::string, std::string>* meta_raw(); // Accesses the metadata map directly. Use with caution!
    std::string name() const;                       // Retrieves the name of thie Item.
    void        new_hex_id();                       // Generates a new hex ID for this Item.
    void        save(std::shared_ptr<SQLite::Database> save_db, uint32_t owner_id); // Saves the Item to the save file.
    void        set_equip_slot(EquipSlot es);       // Sets this Item's equipment slot.
    void        set_meta(const std::string &key, const std::string &value); // Adds Item metadata.
    void        set_name(const std::string &name);  // Sets the name of this Item.
    void        set_tag(ItemTag the_tag);           // Sets a tag on this Item.
    void        set_type(ItemType type, ItemSub sub = ItemSub::NONE);   // Sets the type of this Item.
    ItemSub     subtype() const;                    // Returns the ItemSub (sub-type) of this Item.
    bool        tag(ItemTag the_tag) const;         // Checks if a tag is set on this Item.
    ItemType    type() const;                       // Returns the ItemType of this Item.

private:
    EquipSlot   m_equip_slot;   // The slot this Item is equipped in, if any.
    uint16_t    m_hex_id;       // The hex ID of this Item, for parser differentiation.
    std::map<std::string, std::string>  m_metadata; // The Item's metadata, if any.
    std::string m_name;         // The name of this Item!
    std::set<ItemTag>   m_tags; // Any and all ItemTags on this Item.
    ItemType    m_type;         // The primary type of this Item.
    ItemSub     m_type_sub;     // The subtype of this Item, if any.
};
