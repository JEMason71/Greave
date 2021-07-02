// world/item.hpp -- The Item class is for objects that can be picked up and used by the player or other NPCs.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

namespace SQLite { class Database; }    // defined in 3rdparty/SQLiteCpp/Database.h


// Weapon damage types.
enum class DamageType : int8_t { ACID, BALLISTIC, CRUSHING, EDGED, ENERGY, EXPLOSIVE, KINETIC, PIERCING, PLASMA, POISON, RENDING, NONE = -1 };

// The slot that an item is equipped in.
enum class EquipSlot : uint8_t { NONE, HAND_MAIN, HAND_OFF, BODY, ARMOUR, ABOUT_BODY, HEAD, HANDS, FEET, _END };

// ItemType is the primary type of Item (e.g. weapon, food, etc.)
enum class ItemType : uint16_t { NONE, ARMOUR, KEY, LIGHT, SHIELD, WEAPON };

// ItemSub is for sub-types of items, e.g. a tool could sub-classify itself here.
enum class ItemSub : uint16_t { NONE,
    CLOTHING, HEAVY, LIGHT, MEDIUM, // Armour subtypes.
    MELEE, UNARMED, // Weapon subtypes.
    };

enum class ItemTag : uint16_t {
    HandAndAHalf = 1,   // Hand-and-a-half weapons can be wielded in either one or both hands.
    NoA,                // This Item's name should not be prefaced with 'a' (e.g. 'plate mail armour' instead of 'a plate mail armour').
    OffHandOnly,        // This item can ONLY be equipped in the off-hand.
    PluralName,         // This Item's name is a plural (e.g. "boots").
    PreferOffHand,      // When equipped, this Item prefers to be held in the off-hand.
    ProperNoun,         // This Item's name is a proper noun (e.g. Foehammer).
    TwoHanded,          // This Item requires two hands to wield.
};

class Item
{
public:
                        // Flags for the name() function.
    static const int    NAME_FLAG_A, NAME_FLAG_CAPITALIZE_FIRST, NAME_FLAG_CORE_STATS, NAME_FLAG_ID, NAME_FLAG_FULL_STATS, NAME_FLAG_NO_COLOUR, NAME_FLAG_NO_COUNT, NAME_FLAG_PLURAL,
        NAME_FLAG_THE;
    static const std::string    SQL_ITEMS;  // The SQL table construction string for saving items.

                Item();                             // Constructor, sets default values.
    void        clear_meta(const std::string &key); // Clears a metatag from an Item. Use with caution!
    void        clear_tag(ItemTag the_tag);         // Clears a tag on this Item.
    DamageType  damage_type() const;                // Retrieves this Item's damage type, if any.
    std::string damage_type_string() const;         // Returns a string indicator of this Item's damage type (e.g. edged = E)
    std::string desc() const;                       // Retrieves this Item's description.
    EquipSlot   equip_slot() const;                 // Checks what slot this Item equips in, if any.
    static std::shared_ptr<Item> load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id);  // Loads a new Item from the save file.
    std::string meta(const std::string &key) const; // Retrieves Item metadata.
    std::map<std::string, std::string>* meta_raw(); // Accesses the metadata map directly. Use with caution!
    std::string name(int flags = 0) const;          // Retrieves the name of thie Item.
    void        new_parser_id();                    // Generates a new parser ID for this Item.
    uint16_t    parser_id() const;                  // Retrieves the current ID of this Item, for parser differentiation.
    uint16_t    power() const;                      // Retrieves this Item's power.
    void        save(std::shared_ptr<SQLite::Database> save_db, uint32_t owner_id); // Saves the Item to the save file.
    void        set_description(const std::string &desc);   // Sets this Item's description.
    void        set_equip_slot(EquipSlot es);       // Sets this Item's equipment slot.
    void        set_meta(const std::string &key, const std::string &value); // Adds Item metadata.
    void        set_name(const std::string &name, const std::string &plural_name = ""); // Sets the name of this Item.
    void        set_power(uint16_t power);          // Sets the power of this Item.
    void        set_speed(float speed);             // Sets the speed of this Item.
    void        set_tag(ItemTag the_tag);           // Sets a tag on this Item.
    void        set_type(ItemType type, ItemSub sub = ItemSub::NONE);   // Sets the type of this Item.
    float       speed() const;                      // Retrieves the speed of this Item.
    ItemSub     subtype() const;                    // Returns the ItemSub (sub-type) of this Item.
    bool        tag(ItemTag the_tag) const;         // Checks if a tag is set on this Item.
    ItemType    type() const;                       // Returns the ItemType of this Item.

private:
    int         meta_int(std::string key) const;    // Retrieves metadata, in int format.

    std::string m_description;  // The description of this Item.
    EquipSlot   m_equip_slot;   // The slot this Item is equipped in, if any.
    std::map<std::string, std::string>  m_metadata; // The Item's metadata, if any.
    std::string m_name;         // The name of this Item!
    uint16_t    m_parser_id;    // The semi-unique ID of this Item, for parser differentiation.
    std::string m_plural_name;  // The custom plural name of this Item, if any.
    uint16_t    m_power;        // The power of this Item, if any.
    float       m_speed;        // The speed of this Item, if any.
    std::set<ItemTag>   m_tags; // Any and all ItemTags on this Item.
    ItemType    m_type;         // The primary type of this Item.
    ItemSub     m_type_sub;     // The subtype of this Item, if any.
};
