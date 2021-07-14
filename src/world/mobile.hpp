// world/mobile.hpp -- The Mobile class defines entities that can move and interact with the game world.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Inventory;                // defined in world/inventory.hpp
enum class EquipSlot : uint8_t; // defined in world/item.hpp
namespace SQLite { class Database; class Statement; }   // defined in 3rdparty/SQLiteCpp/Database.h

enum class Gender : uint8_t { FEMALE, MALE, IT, THEY };

enum class CombatStance : uint8_t { BALANCED, AGGRESSIVE, DEFENSIVE };

enum class MobileTag : uint16_t { None = 0,

    // Tags that affect the Mobile's name.
    PluralName,         // This Mobile's name is a plural (e.g. "pack of rats").
    ProperNoun,         // This Mobile's name is a proper noun (e.g. Smaug).

    // Tags that affect the Mobile's abilities or stats in combat.
    CannotBlock,        // This Mobile is unable to block attacks.
    CannotDodge,        // This Mobile is unable to dodge attacks.
    CannotParry,        // This Mobbile is unable to parry melee attacks.
    Agile,              // Agile Mobiles are harder to hit in melee combat (0.8x hit chance), and more likely to parry attacks (1.5x chance).
    Clumsy,             // The opposite of Agile, Clumsy Mobiles are easier to hit in melee (1.25x hit chance) and suffer a penalty to parrying (0.5x chance).
    Anemic,             // Anemic Mobiles deal less damage with melee attacks (0.5x).
    Feeble,             // Feeble Mobiles deal less damage with melee attacks (0.75x).
    Puny,               // Puny Mobiles deal less damage with melee attacks (0.9x).
    Strong,             // Strong Mobiles deal more damage with melee attacks (1.1x).
    Brawny,             // Brawny Mobiles deal more damage with melee attacks (1.25x).
    Vigorous,           // Vigorous Mobiles deal more damage with melee attacks (1.5x).
    Mighty,             // Mighty Mobiles deal more damage with melee attacks (2x).

    // Tags that determine the Mobile's general state of being.
    Beast,              // This Mobile is a beast or creature; it has body-parts rather than equipment.
    ImmunityBleed,      // This Mobile is unable to bleed.
    ImmunityPoison,     // This Mobile is immune to being poisoned.
    RandomGender,       // This Mobile can be assigned a random gender.
    Unliving,           // This Mobile is a construct or other unliving entity.

    // Tags regarding the Mobile's AI and behaviour.
    AggroOnSight,       // This Mobile will attack the player on sight.
    CannotOpenDoors,    // This Mobile cannot open doors.
    Coward,             // This Mobile will try to run rather than fight.
    Resting,            // This Mobile is currently resting.

    // Temporary tags assigned by the game.
    ArenaFighter,       // This Mobile is your opponent in an arena fight. (Or, when set on the player, they are currently engaged in an arena fight.)
};

struct BodyPart
{
    uint8_t     hit_chance; // The hit chance for this body part.
    std::string name;       // The name of this body part.
    EquipSlot   slot;       // The EquipSlot associated with this body part.
};

struct Buff
{
    enum class Type : uint8_t { NONE, BLEED, POISON, RECENT_DAMAGE, RECENTLY_FLED };

    static const std::string    SQL_BUFFS;  // The SQL table construction string for Buffs.

    static std::shared_ptr<Buff>    load(SQLite::Statement &query); // Loads this Buff from a save file.
    void    save(std::shared_ptr<SQLite::Database> save_db, uint32_t owner_id); // Saves this Buff to a save file.

    uint32_t    power = 0;              // The power level of this buff/debuff.
    uint16_t    time = USHRT_MAX;       // The time remaining on this buff/debuff, or USHRT_MAX for effects that expire on special circumstances.
    Type        type = Type::NONE;      // The type of buff/debuff.
};


class Mobile
{
public:
                                // Flags for the name() function.
    static const int            NAME_FLAG_A, NAME_FLAG_CAPITALIZE_FIRST, NAME_FLAG_NO_COLOUR, NAME_FLAG_HEALTH, NAME_FLAG_PLURAL, NAME_FLAG_POSSESSIVE, NAME_FLAG_THE;
    static const std::string    SQL_MOBILES;    // The SQL table construction string for Mobiles.

                        Mobile();                                   // Constructor, sets default values.
    void                add_hostility(uint32_t mob_id);             // Adds a Mobile (or the player, with ID 0) to this Mobile's hostility list.
    void                add_second();                               // Adds a second to this Mobile's action timer.
    void                add_score(int score);                       // Adds to this Mobile's score.
    float               attack_speed() const;                       // Returns the number of seconds needed for this Mobile to make an attack.
    float               block_mod() const;                          // Returns the modified chance to block for this Mobile, based on equipped gear.
    uint32_t            buff_power(Buff::Type type) const;          // Returns the power level of the specified buff/debuff.
    uint16_t            buff_time(Buff::Type type) const;           // Returns the time remaining for the specifieid buff/debuff.
    bool                can_perform_action(float time) const;       // Checks if this Mobile has enough action timer built up to perform an action.
    uint32_t            carry_weight() const;                       // Checks how much weight this Mobile is carrying.
    void                clear_buff(Buff::Type type);                // Clears a specified buff/debuff from the Actor, if it exists.
    void                clear_meta(const std::string &key);         // Clears a metatag from a Mobile. Use with caution!
    void                clear_tag(MobileTag the_tag);               // Clears an MobileTag from this Mobile.
    float               dodge_mod() const;                          // Returns the modified chance to dodge for this Mobile, based on equipped gear.
    const std::shared_ptr<Inventory>    equ() const;                // Returns a pointer to the Movile's equipment.
    const std::vector<std::shared_ptr<BodyPart>>& get_anatomy() const;  // Retrieves the anatomy vector for this Mobile.
    bool                has_buff(Buff::Type type) const;            // Checks if this Actor has the specified buff/debuff active.
    std::string         he_she() const;                             // Returns a gender string (he/she/it/they/etc.)
    std::string         his_her() const;                            // Returns a gender string (his/her/its/their/etc.)
    const std::vector<uint32_t>&    hostility_vector() const;       // Returns the hostility vector.
    int                 hp(bool max = false) const;                 // Retrieves the HP (or maximum HP) of this Mobile.
    uint32_t            id() const;                                 // Retrieves the unique ID of this Mobile.
    const std::shared_ptr<Inventory>    inv() const;                // Returns a pointer to the Mobile's Inventory.
    virtual bool        is_dead() const;                            // Checks if this Mobile is dead.
    bool                is_hostile() const;                         // Is this Mobile hostile to the player?
    virtual bool        is_player() const;                          // Returns true if this Mobile is a Player, false if not.
    virtual uint32_t    load(std::shared_ptr<SQLite::Database> save_db, uint32_t sql_id);   // Loads a Mobile.
    uint32_t            location() const;                           // Retrieves the location of this Mobile, in the form of a Room ID.
    uint32_t            max_carry() const;                          // The maximum weight this Mobile can carry.
    std::string         meta(const std::string &key) const;         // Retrieves Mobile metadata.
    float               meta_float(const std::string &key) const;   // Retrieves metadata, in float format.
    int                 meta_int(const std::string &key) const;     // Retrieves metadata, in int format.
    uint32_t            meta_uint(const std::string &key) const;    // Retrieves metadata, in unsigned 32-bit integer format.
    std::map<std::string, std::string>* meta_raw();                 // Accesses the metadata map directly. Use with caution!
    std::string         name(int flags = 0) const;                  // Retrieves the name of this Mobile.
    void                new_parser_id();                            // Generates a new parser ID for this Mobile.
    float               parry_mod() const;                          // Returns the modified chance to parry for this Mobile, based on equipped gear.
    uint16_t            parser_id() const;                          // Retrieves the current ID of this Mobile, for parser differentiation.
    bool                pass_time(float seconds = 0.0f, bool interruptable = true); // Causes time to pass for this Mobile.
    virtual void        reduce_hp(int amount, bool death_message = true);   // Reduces this Mobile's hit points.
    int                 restore_hp(int amount);                     // Restores a specified amount of hit points.
    virtual uint32_t    save(std::shared_ptr<SQLite::Database> save_db);    // Saves this Mobile.
                        // Sets a specified buff/debuff on the Actor, or extends an existing buff/debuff.
    uint32_t            score() const;                              // Checks this Mobile's score.
    void                set_buff(Buff::Type type, uint16_t time = USHRT_MAX, uint32_t power = 0, bool additive_power = false, bool additive_time = true);
    void                set_gender(Gender gender);                  // Sets the gender of this Mobile.
    void                set_hp(int hp, int hp_max = 0);             // Sets the current (and, optionally, maximum) HP of this Mobile.
    void                set_id(uint32_t new_id);                    // Sets this Mobile's unique ID.
    void                set_location(uint32_t room_id);             // Sets the location of this Mobile with a Room ID.
    void                set_location(const std::string &room_id);   // As above, but with a string Room ID.
    void                set_meta(const std::string &key, std::string value);    // Adds Item metadata.
    void                set_meta(const std::string &key, int value);            // As above, but with an integer value.
    void                set_meta(const std::string &key, float value);          // As above again, but this time for floats.
    void                set_meta_uint(const std::string &key, uint32_t value);  // As above, but with an unsigned 32-bit integer.
    void                set_name(const std::string &name);          // Sets the name of this Mobile.
    void                set_spawn_room(uint32_t id);                // Sets this Mobile's spawn room.
    void                set_species(const std::string &species);    // Sets the species of this Mobile.
    void                set_stance(CombatStance stance);            // Sets this Mobile's combat stance.
    void                set_tag(MobileTag the_tag);                 // Sets a MobileTag on this Mobile.
    std::string         species() const;                            // Checks the species of this Mobile.
    CombatStance        stance() const;                             // Checks this Mobile's combat stance.
    bool                tag(MobileTag the_tag) const;               // Checks if a MobileTag is set on this Mobile.
    bool                tick_bleed(uint32_t power, uint16_t time);  // Triggers a single bleed tick.
    void                tick_buffs();                               // Reduce the timer on all buffs.
    virtual void        tick_hp_regen();                            // Regenerates HP over time.
    bool                tick_poison(uint32_t power, uint16_t time); // Triggers a single poison tick.

protected:
    static const float  ACTION_TIMER_CAP_MAX;                   // The maximum value the action timer can ever reach.
    static const int    BASE_CARRY_WEIGHT;                      // The maximum amount of weight a Mobile can carry, before modifiers.
    static const int    DAMAGE_DEBUFF_TIME;                     // How long the damage debuff that prevents HP regeneration lasts.
    static const int    SCAR_BLEED_INTENSITY_FROM_BLEED_TICK;   // Blood type scar intensity caused by each tick of the player or an NPC bleeding.

    std::shared_ptr<Buff>   buff(Buff::Type type) const;    // Returns a pointer to a specified Buff.

    float                               m_action_timer; // 'Charges up' with time, to allow NPCs to perform timed actions.
    std::vector<std::shared_ptr<Buff>>  m_buffs;        // Any and all buffs or debuffs on this Mobile.
    std::shared_ptr<Inventory>          m_equipment;    // The Items currently worn or wielded by this Mobile.
    Gender                              m_gender;       // The gender of this Mobile.
    std::vector<uint32_t>               m_hostility;    // The hostility vector keeps track of who this Mobile is angry with.
    int                                 m_hp[2];        // The current and maxmum hit points of this Mobile.
    uint32_t                            m_id;           // The Mobile's unique ID.
    std::shared_ptr<Inventory>          m_inventory;    // The Items being carried by this Mobile.
    uint32_t                            m_location;     // The Room that this Mobile is currently located in.
    std::map<std::string, std::string>  m_metadata;     // The Mobile's metadata, if any.
    std::string                         m_name;         // The name of this Mobile.
    uint16_t                            m_parser_id;    // The semi-unique ID of this Mobile, for parser differentiation.
    uint32_t                            m_score;        // Either the score value for killing this Mobile; or, for the Player, their current total score.
    uint32_t                            m_spawn_room;   // The Room that spawned this Mobile.
    std::string                         m_species;      // Ths species type of this Mobile.
    CombatStance                        m_stance;       // The Mobile's current combat stance.
    std::set<MobileTag>                 m_tags;         // Any and all tags on this Mobile.
};
