// world/mobile.hpp -- The Mobile class defines entities that can move and interact with the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Inventory;                        // defined in world/inventory.hpp
enum class EquipSlot : uint8_t;         // defined in world/item.hpp
namespace SQLite { class Database; }    // defined in 3rdparty/SQLiteCpp/Database.h


enum class Gender : uint8_t { FEMALE, MALE, IT, THEY };

enum class MobileTag : uint16_t { None = 0,

    // Tags that affect the Mobile's name.
    PluralName,     // This Mobile's name is a plural (e.g. "pack of rats").
    ProperNoun,     // This Mobile's name is a proper noun (e.g. Smaug).

    // Tags that affect the Mobile's abilities or stats in combat.
    CannotBlock,    // This Mobile is unable to block attacks.
    CannotDodge,    // This Mobile is unable to dodge attacks.
    CannotParry,    // This Mobbile is unable to parry melee attacks.
    Agile,          // Agile Mobiles are harder to hit in combat, and more likely to parry attacks.
    Clumsy,         // The opposite of Agile, Clumsy Mobiles are easier to hit and suffer a penalty to parrying.

    // Tags that determine the Mobile's general state of being.
    Unliving,       // This Mobile is a construct or other unliving entity.

    // Tags regarding the Mobile's AI and behaviour.
    AggroOnSight,   // This Mobile will attack the player on sight.
    Coward,         // This Mobile will try to run rather than fight.
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
    static const std::string    SQL_MOBILES;    // The SQL table construction string for Mobiles.

                        Mobile();                                   // Constructor, sets default values.
    void                add_hostility(uint32_t mob_id);             // Adds a Mobile (or the player, with ID 0) to this Mobile's hostility list.
    void                add_second();                               // Adds a second to this Mobile's action timer.
    float               attack_speed() const;                       // Returns the number of seconds needed for this Mobile to make an attack.
    float               block_mod() const;                          // Returns the modified chance to block for this Mobile, based on equipped gear.
    bool                can_perform_action(float time) const;       // Checks if this Mobile has enough action timer built up to perform an action.
    uint32_t            carry_weight() const;                       // Checks how much weight this Mobile is carrying.
    void                clear_tag(MobileTag the_tag);               // Clears an MobileTag from this Mobile.
    float               dodge_mod() const;                          // Returns the modified chance to dodge for this Mobile, based on equipped gear.
    const std::shared_ptr<Inventory>    equ() const;                // Returns a pointer to the Movile's equipment.
    const std::vector<std::shared_ptr<BodyPart>>& get_anatomy() const;  // Retrieves the anatomy vector for this Mobile.
    std::string         he_she() const;                             // Returns a gender string (he/she/it/they/etc.)
    std::string         his_her() const;                            // Returns a gender string (his/her/its/their/etc.)
    const std::vector<uint32_t>&  hostility_vector() const;         // Returns the hostility vector.
    int                 hp(bool max = false) const;                 // Retrieves the HP (or maximum HP) of this Mobile.
    uint32_t            id() const;                                 // Retrieves the unique ID of this Mobile.
    const std::shared_ptr<Inventory>    inv() const;                // Returns a pointer to the Mobile's Inventory.
    bool                is_dead() const;                            // Checks if this Mobile is dead.
    bool                is_hostile() const;                         // Is this Mobile hostile to the player?
    virtual bool        is_player() const;                          // Returns true if this Mobile is a Player, false if not.
    virtual uint32_t    load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id);   // Loads a Mobile.
    uint32_t            location() const;                           // Retrieves the location of this Mobile, in the form of a Room ID.
    uint32_t            max_carry() const;                          // The maximum weight this Mobile can carry.
    std::string         name(int flags = 0) const;                  // Retrieves the name of this Mobile.
    void                new_parser_id();                            // Generates a new parser ID for this Mobile.
    float               parry_mod() const;                          // Returns the modified chance to parry for this Mobile, based on equipped gear.
    uint16_t            parser_id() const;                          // Retrieves the current ID of this Mobile, for parser differentiation.
    bool                pass_time(float seconds = 0.0f);            // Causes time to pass for this Mobile.
    void                reduce_hp(int amount);                      // Reduces this Mobile's hit points.
    int                 restore_hp(int amount);                     // Restores a specified amount of hit points.
    virtual uint32_t    save(std::shared_ptr<SQLite::Database> save_db);    // Saves this Mobile.
    void                set_hp(int hp, int hp_max = 0);             // Sets the current (and, optionally, maximum) HP of this Mobile.
    void                set_id(uint32_t new_id);                    // Sets this Mobile's unique ID.
    void                set_location(uint32_t room_id);             // Sets the location of this Mobile with a Room ID.
    void                set_location(const std::string &room_id);   // As above, but with a string Room ID.
    void                set_name(const std::string &name);          // Sets the name of this Mobile.
    void                set_spawn_room(uint32_t id);                // Sets this Mobile's spawn room.
    void                set_species(const std::string &species);    // Sets the species of this Mobile.
    void                set_tag(MobileTag the_tag);                 // Sets a MobileTag on this Mobile.
    std::string         species() const;                            // Checks the species of this Mobile.
    bool                tag(MobileTag the_tag) const;               // Checks if a MobileTag is set on this Mobile.

protected:
    static const float      ACTION_TIMER_CAP_MAX;   // The maximum value the action timer can ever reach.
    static const uint32_t   BASE_CARRY_WEIGHT;      // The maximum amount of weight a Mobile can carry, before modifiers.

    float                       m_action_timer; // 'Charges up' with time, to allow NPCs to perform timed actions.
    std::shared_ptr<Inventory>  m_equipment;    // The Items currently worn or wielded by this Mobile.
    Gender                      m_gender;       // The gender of this Mobile.
    std::vector<uint32_t>       m_hostility;    // The hostility vector keeps track of who this Mobile is angry with.
    int                         m_hp[2];        // The current and maxmum hit points of this Mobile.
    uint32_t                    m_id;           // The Mobile's unique ID.
    std::shared_ptr<Inventory>  m_inventory;    // The Items being carried by this Mobile.
    uint32_t                    m_location;     // The Room that this Mobile is currently located in.
    std::string                 m_name;         // The name of this Mobile.
    uint16_t                    m_parser_id;    // The semi-unique ID of this Mobile, for parser differentiation.
    uint32_t                    m_spawn_room;   // The Room that spawned this Mobile.
    std::string                 m_species;      // Ths species type of this Mobile.
    std::set<MobileTag>         m_tags;         // Any and all tags on this Mobile.
};
