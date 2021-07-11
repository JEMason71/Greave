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
enum class ItemType : uint16_t { NONE, AMMO, ARMOUR, KEY, LIGHT, SHIELD, WEAPON };

// ItemSub is for sub-types of items, e.g. a tool could sub-classify itself here.
enum class ItemSub : uint16_t { NONE,
    ARROW, BOLT,                    // Ammo subtypes.
    CLOTHING, HEAVY, LIGHT, MEDIUM, // Armour subtypes.
    MELEE, RANGED, UNARMED,         // Weapon subtypes.
    };

enum class ItemTag : uint16_t { _None = 0,  // Do not use this tag, it's just a marker to start the tags below counting from 1.

    // Basic, generic, core details about an item.
    Stackable,          // Can this Item be stacked with others identical to itself?

    // Tags regarding the item's name.
    NoA,                // This Item's name should not be prefaced with 'a' (e.g. 'plate mail armour' instead of 'a plate mail armour').
    PluralName,         // This Item's name is a plural (e.g. "boots").
    ProperNoun,         // This Item's name is a proper noun (e.g. Foehammer).

    // Tags specific to weapons, armour and other equipment.
    AmmoArrow,          // Either this is a weapon that uses arrows, or it's a stack of arrows.
    AmmoBolt,           // Either this is a weapon that uses bolts, or it's a stack of bolts.
    HandAndAHalf,       // Hand-and-a-half weapons can be wielded in either one or both hands.
    NoAmmo,             // This is a ranged weapon that does not require ammo.
    OffHandOnly,        // This item can ONLY be equipped in the off-hand.
    PreferOffHand,      // When equipped, this Item prefers to be held in the off-hand.
    TwoHanded,          // This Item requires two hands to wield.
};

class Item
{
public:
                        // Flags for the name() function.
    static const int    NAME_FLAG_A, NAME_FLAG_CAPITALIZE_FIRST, NAME_FLAG_CORE_STATS, NAME_FLAG_ID, NAME_FLAG_FULL_STATS, NAME_FLAG_NO_COLOUR, NAME_FLAG_NO_COUNT, NAME_FLAG_PLURAL,
        NAME_FLAG_RARE, NAME_FLAG_THE;
    static const std::string    SQL_ITEMS;  // The SQL table construction string for saving items.

                Item();                                     // Constructor, sets default values.
    float       ammo_power() const;                         // The damage multiplier for ammunition.
    float       armour(int bonus_power = 0) const;          // Returns the armour damage reduction value of this Item, if any.
    int         bleed() const;                              // Returns thie bleed chance of this Item, if any.
    int         block_mod() const;                          // Returns the block modifier% for this Item, if any.
    void        clear_meta(const std::string &key);         // Clears a metatag from an Item. Use with caution!
    void        clear_tag(ItemTag the_tag);                 // Clears a tag on this Item.
    int         crit() const;                               // Retrieves this Item's critical power, if any.
    DamageType  damage_type() const;                        // Retrieves this Item's damage type, if any.
    std::string damage_type_string() const;                 // Returns a string indicator of this Item's damage type (e.g. edged = E)
    std::string desc() const;                               // Retrieves this Item's description.
    int         dodge_mod() const;                          // Returns the dodge modifier% for this Item, if any.
    EquipSlot   equip_slot() const;                         // Checks what slot this Item equips in, if any.
    bool        is_identical(std::shared_ptr<Item> item) const; // Checks if this Item is identical to another (except stack size).
    static std::shared_ptr<Item> load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id);  // Loads a new Item from the save file.
    std::string meta(const std::string &key) const;         // Retrieves Item metadata.
    float       meta_float(const std::string &key) const;   // Retrieves metadata, in float format.
    int         meta_int(const std::string &key) const;     // Retrieves metadata, in int format.
    std::map<std::string, std::string>* meta_raw();         // Accesses the metadata map directly. Use with caution!
    std::string name(int flags = 0) const;                  // Retrieves the name of thie Item.
    void        new_parser_id();                            // Generates a new parser ID for this Item.
    int         parry_mod() const;                          // Returns the parry% modifier of this Item, if any.
    uint16_t    parser_id() const;                          // Retrieves the current ID of this Item, for parser differentiation.
    int         poison() const;                             // Returns the poison chance of this item, if any.
    int         power() const;                              // Retrieves this Item's power.
    int         rare() const;                               // Retrieves this Item's rarity.
    void        save(std::shared_ptr<SQLite::Database> save_db, uint32_t owner_id); // Saves the Item to the save file.
    void        set_description(const std::string &desc);   // Sets this Item's description.
    void        set_equip_slot(EquipSlot es);               // Sets this Item's equipment slot.
    void        set_meta(const std::string &key, std::string value);    // Adds Item metadata.
    void        set_meta(const std::string &key, int value);            // As above, but with an integer value.
    void        set_meta(const std::string &key, float value);          // As above again, but this time for floats.
    void        set_name(const std::string &name);          // Sets the name of this Item.
    void        set_rare(int rarity);                       // Sets this Item's rarity.
    void        set_stack(uint32_t size);                   // Sets the stack size for this Item.
    void        set_tag(ItemTag the_tag);                   // Sets a tag on this Item.
    void        set_type(ItemType type, ItemSub sub = ItemSub::NONE);   // Sets the type of this Item.
    void        set_value(uint32_t val);                    // Sets this Item's value.
    void        set_weight(uint32_t pacs);                  // Sets this Item's weight.
    float       speed() const;                              // Retrieves the speed of this Item.
    std::shared_ptr<Item>    split(int split_count);        // Splits an Item into a stack.
    uint32_t    stack() const;                              // Retrieves the stack size of this Item.
    std::string stack_name(int stack_size, int flags = 0);  // Like name(), but provides an appropriate name for a given stack size. Works on non-stackable items too.
    ItemSub     subtype() const;                            // Returns the ItemSub (sub-type) of this Item.
    bool        tag(ItemTag the_tag) const;                 // Checks if a tag is set on this Item.
    ItemType    type() const;                               // Returns the ItemType of this Item.
    uint32_t    value(bool individual = false) const;       // The Item's value in money.
    int         warmth() const;                             // The Item's warmth rating, if any.
    uint32_t    weight(bool individual = false) const;      // The Item's weight, in pacs.

private:
    std::string m_description;      // The description of this Item.
    std::map<std::string, std::string>  m_metadata; // The Item's metadata, if any.
    std::string m_name;             // The name of this Item!
    uint16_t    m_parser_id;        // The semi-unique ID of this Item, for parser differentiation.
    uint8_t     m_rarity;           // The rarity of this Item.
    uint32_t    m_stack;            // If this Item can be stacked, this is how many is in the stack.
    std::set<ItemTag>   m_tags;     // Any and all ItemTags on this Item.
    ItemType    m_type;             // The primary type of this Item.
    ItemSub     m_type_sub;         // The subtype of this Item, if any.
    uint32_t    m_value;            // The value of this Item, if any.
    uint32_t    m_weight;           // The weight of this Item.
};
