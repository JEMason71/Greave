// world/mobile.hpp -- The Mobile class defines entities that can move and interact with the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Inventory;                        // defined in world/inventory.hpp
enum class EquipSlot : uint8_t;         // defined in world/item.hpp
namespace SQLite { class Database; }    // defined in 3rdparty/SQLiteCpp/Database.h


enum class Gender : uint8_t { FEMALE, MALE, IT, THEY };

enum class MobileTag : uint16_t {
    PluralName = 1, // This Mobile's name is a plural (e.g. "pack of rats").
    ProperNoun,     // This Mobile's name is a proper noun (e.g. Smaug).
    Unliving,       // This Mobile is a construct or other unliving entity.
};

struct BodyPart
{
    uint8_t     hit_chance; // The hit chance for this body part.
    std::string name;       // The name of this body part.
    EquipSlot   slot;       // The EquipSlot associated with this body part.
};

class Mobile
{
public:
                                // Flags for the name() function.
    static const int            NAME_FLAG_A, NAME_FLAG_CAPITALIZE_FIRST, NAME_FLAG_NO_COLOUR, NAME_FLAG_PLURAL, NAME_FLAG_POSSESSIVE, NAME_FLAG_THE;
    static const std::string    SQL_MOBILES;        // The SQL table construction string for Mobiles.

                        Mobile();                                   // Constructor, sets default values.
    float               attack_speed() const;                       // Returns the number of seconds needed for this Mobile to make an attack.
    void                clear_tag(MobileTag the_tag);               // Clears an MobileTag from this Mobile.
    const std::shared_ptr<Inventory>    equ() const;                // Returns a pointer to the Movile's equipment.
    const std::vector<std::shared_ptr<BodyPart>>& get_anatomy() const;  // Retrieves the anatomy vector for this Mobile.
    std::string         he_she() const;                             // Returns a gender string (he/she/it/they/etc.)
    std::string         his_her() const;                            // Returns a gender string (his/her/its/their/etc.)
    int                 hp(bool max = false) const;                 // Retrieves the HP (or maximum HP) of this Mobile.
    const std::shared_ptr<Inventory>    inv() const;                // Returns a pointer to the Mobile's Inventory.
    bool                is_dead() const;                            // Checks if this Mobile is dead.
    virtual bool        is_player() const;                          // Returns true if this Mobile is a Player, false if not.
    virtual uint32_t    load(std::shared_ptr<SQLite::Database> save_db, unsigned int sql_id);   // Loads a Mobile.
    uint32_t            location() const;                           // Retrieves the location of this Mobile, in the form of a Room ID.
    std::string         name(int flags = 0) const;                  // Retrieves the name of this Mobile.
    void                new_parser_id();                            // Generates a new parser ID for this Mobile.
    uint16_t            parser_id() const;                          // Retrieves the current ID of this Mobile, for parser differentiation.
    bool                pass_time(float seconds);                   // Causes time to pass for this Mobile.
    void                reduce_hp(int amount);                      // Reduces this Mobile's hit points.
    int                 restore_hp(int amount);                     // Restores a specified amount of hit points.
    virtual uint32_t    save(std::shared_ptr<SQLite::Database> save_db);    // Saves this Mobile.
    void                set_hp(int hp, int hp_max = 0);             // Sets the current (and, optionally, maximum) HP of this Mobile.
    void                set_location(uint32_t room_id);             // Sets the location of this Mobile with a Room ID.
    void                set_location(const std::string &room_id);   // As above, but with a string Room ID.
    void                set_name(const std::string &name);          // Sets the name of this Mobile.
    void                set_species(const std::string &species);    // Sets the species of this Mobile.
    void                set_tag(MobileTag the_tag);                 // Sets a MobileTag on this Mobile.
    std::string         species() const;                            // Checks the species of this Mobile.
    bool                tag(MobileTag the_tag) const;               // Checks if a MobileTag is set on this Mobile.

private:
    float               m_action_timer; // When this timer reaches 0, the Mobile is able to act. Any actions it takes detract from the timer.
    std::shared_ptr<Inventory>  m_equipment;    // The Items currently worn or wielded by this Mobile.
    Gender              m_gender;       // The gender of this Mobile.
    int                 m_hp[2];        // The current and maxmum hit points of this Mobile.
    std::shared_ptr<Inventory>  m_inventory;    // The Items being carried by this Mobile.
    uint32_t            m_location;     // The Room that this Mobile is currently located in.
    std::string         m_name;         // The name of this Mobile.
    uint16_t            m_parser_id;    // The semi-unique ID of this Mobile, for parser differentiation.
    std::string         m_species;      // Ths species type of this Mobile.
    std::set<MobileTag> m_tags;         // Any and all tags on this Mobile.
};
