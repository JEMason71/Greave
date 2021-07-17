// world/room.h -- The Room class, which defines a single area in the game world that the player can visit.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_WORLD_ROOM_H_
#define GREAVE_WORLD_ROOM_H_

#include <cstddef>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "world/inventory.h"


namespace greave {

enum class Direction : uint8_t { NORTH, SOUTH, EAST, WEST, NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST, UP, DOWN, NONE };

enum class ScarType : uint8_t { BLOOD, BURN, DEBRIS, DIRT, VOMIT, CAMPFIRE, WATER };

enum class Security : uint8_t { ANARCHY, LOW, HIGH, SANCTUARY, INACCESSIBLE };

enum class LinkTag : uint16_t {
    // ****************************************************************************************
    // Tags below 10,000 are considered *dynamic tags*. These tags WILL be saved to save files.
    // ****************************************************************************************
    _None = 0,          // Do not use this tag, it's just a marker to start the tags below counting from 1.

    // Link tags regarding doors and other openable/lockable exits.
    Open,               // A door, gate, hatch, etc. that is currently open.
    Locked,             // A closed door etc. that is currently locked.
    Unlocked,           // As above, but one that is explicitly unlocked.
    KnownLocked,        // The player is aware that this exit is locked.
    TempPermalock,      // Like Permalock, but a temporary version.

    // ****************************************************************************************************
    // Tags at 10,000 or above are considered *permanent tags*. These tags WILL NOT be saved to save files.
    // ****************************************************************************************************
    _Permanent = 9999,  // Do not use this tag, it's just a marker to start the tags below counting from 10,000.

    // Basic properties of links, such as their length.
    Hidden,             // This link is not normally visible.
    DoubleLength,       // This link is twice as long, so will take extra time to traverse.
    TripleLength,       // This link is three times as long, so will take extra time to traverse.
    Incline,            // This link also leads upward. [CURRENTLY UNUSED]
    Decline,            // This link also leads downward. [CURRENTLY UNUSED]

    // Link tags regarding doors and other openable/lockable exits.
    Openable,           // Can be opened and closed; a door, hatch, gate, or similar.
    Lockable,           // Can be locked or unlocked with a key. Must be openable.
    LockedByDefault,    // This exit is locked by default (requires the Unlocked tag to be considered unlocked).
    LockWeak,           // Does this door have a particularly weak lock? [CURRENTLY UNUSED]
    LockStrong,         // Does this door have a particularly strong lock? [CURRENTLY UNUSED]
    Permalock,          // A locked door/etc. which can never be unlocked.
    DoorMetal,          // Is the door on this link metal?
    Window,             // This 'door' is actually a window.
    DoorShop,           // This is the door to a shop and should be locked at night. [CURRENTLY UNUSED]
    AutoClose,          // This door will automatically close after the player passes through it. [CURRENTLY UNUSED]
    AutoLock,           // This door will automatically lock after the player passes through it. [CURRENTLY UNUSED]
    LocksWhenClosed,    // When this door is closed, it will automatically lock itself. [CURRENTLY UNUSED]

    // Links that lead to special room types.
    Ocean,              // For links that go into the ocean. [CURRENTLY UNUSED]
    Sky,                // This link leads to a virtual sky room, with the actual linked room below.
    Sky2,               // As above, but the sky room is stacked two rooms high.
    Sky3,               // As above, but three rooms high.

    // Link tags regarding NPC behaviour.
    NoMobRoam,          // NPCs should not roam through this exit. [CURRENTLY UNUSED]
    NoBlockExit,        // NPCs should not block the player passing this way, even if they're going to a private room. [CURRENTLY UNUSED]
};

enum class RoomTag : uint16_t {

    // ****************************************************************************************
    // Tags below 10,000 are considered *dynamic tags*. These tags WILL be saved to save files.
    // ****************************************************************************************
    _None = 0,              // Do not use this tag, it's just a marker to start the tags below counting from 1.

    Explored,               // The player has visited this room before.
    MetaChanged,            // The metadata on this room has changed.
    MobSpawned,             // This room has spawned a mob already.
    MobSpawnListChanged,    // The mob spawn list on this room has changed.
    SaveActive,             // An extra-temporary tag used by the save system, to keep track of active rooms when saving/loading the game.


    // ****************************************************************************************************
    // Tags at 10,000 or above are considered *permanent tags*. These tags WILL NOT be saved to save files.
    // ****************************************************************************************************
    _Permanent = 9999,      // Do not use this tag, it's just a marker to start the tags below counting from 10,000.

    // Basic, core properties of rooms, such as if they're indoors.
    Indoors,                // Is this room indoors?
    Underground,            // Is this room underground?
    CanSeeOutside,          // Are we indoors, but can see the weather outside?
    DigOK,                  // The ground here can be dug up easily with a shovel. [CURRENTLY UNUSED]
    Wide,                   // Is this room quite spacious? [CURRENTLY UNUSED]
    VeryWide,               // Is this room extremely spacious? [CURRENTLY UNUSED]
    NoExploreCredit,        // This room does not count towards the number of 'explored' rooms in the player's stats. [CURRENTLY UNUSED]
    Trees,                  // There are trees in this area.
    SleepOK,                // Environmental noises will affect the player much less here. [CURRENTLY UNUSED]
    HeatedInterior,         // Is this interior area heated?
    PermaCampfire,          // Treat this room like it always has a campfire burning.
    HideCampfireScar,       // This is a bit specific. It's for rooms with PermaCampfire, where we don't want the campfire 'scar' text showing.

    // Tags regarding the presence of water in this room.
    WaterClean,             // There is a source of clean water nearby.
    WaterTainted,           // There is a water source nearby, but it is unclean. [CURRENTLY UNUSED]
    WaterSalt,              // There is a source of salt water nearby. [CURRENTLY UNUSED]
    WaterShallow,           // Is this shallow water, the kind that doesn't require swimming?
    WaterDeep,              // Is this deep water, which can be drowned in?

    // Tags regarding NPC behaviour and crime.
    Private,                // Entering this room is considered trespassing. [CURRENTLY UNUSED]

    // Tags regarding shops.
    Shop,                   // Khajiit has wares, if you have coin.
    ShopBuysContraband,     // This room's shop is willing to buy stolen/contraband items. [CURRENTLY UNUSED]
    ShopRespawningOwner,    // This shop's owner NPC respawns or is replaced. [CURRENTLY UNUSED]

    // Tags involving the sanity system.
    Gross,                  // This room is disgusting, and can trigger sanity effects. [CURRENTLY UNUSED]
    Smelly,                 // As with the Gross tag, but in this case the effect is triggered by smell, not sight. [CURRENTLY UNUSED]

    // Special types of rooms.
    Arena,                  // This room contains an arena, where you can fight for money.
    ChurchAltar,            // This room is a church altar, we can respawn here. [CURRENTLY UNUSED]
    GamePoker,              // This room contains a video poker minigame. [CURRENTLY UNUSED]
    GameSlots,              // This room contains a slot machine minigame. [CURRENTLY UNUSED]
    Maze,                   // This Room is part of a maze, and should not have its exit names labeled.
    Nexus,                  // This room contains a nexus transportation system. [CURRENTLY UNUSED]
    RadiationLight,         // This area is lightly irradiated. [CURRENTLY UNUSED]
    SludgePit,              // We got a sinky sludge pit here, guys. [CURRENTLY UNUSED]
    Tavern,                 // This room is a tavern, or part of a tavern.
};

class Room
{
public:
    static const uint32_t       BLOCKED;                // Hashed value for BLOCKED, which is used to mark exits as impassible.
    static const uint32_t       FALSE_ROOM;             // Hashed value for FALSE_ROOM, which is used to make 'fake' impassible room exits.
    static const uint8_t        LIGHT_VISIBLE;          // Any light level below this is considered too dark to see.
    static const size_t         NO_CAMPFIRE;            // Special variable to indicate there is no campfire present here.
    static const int            ROOM_LINKS_MAX = 10;    // The maximum amount of exit links from one Room to another.
    static const std::string    SQL_ROOMS;              // The SQL table construction string for the saved rooms.
    static const uint32_t       UNFINISHED;             // Hashed value for UNFINISHED, which is used to mark room exits as unfinished and to be completed later.

    static const uint32_t       TEMPERATURE_FLAG_WITH_PLAYER_BUFFS;     // Apply the Player's buffs to the result of the room's temperature() calculations.
    static const uint32_t       TEMPERATURE_FLAG_IGNORE_LINKED_ROOMS;   // Calculate a room's temperature() without taking adjacent rooms into account.
    static const uint32_t       TEMPERATURE_FLAG_IGNORE_PLAYER_CLOTHES; // Ignore the Player's clothing when calculating temperature.

                Room(std::string new_id = "");                          // Constructor, sets the Room's ID hash.
    void        activate();                                             // This Room was previously inactive, and has now become active.
    void        add_scar(ScarType type, int intensity);                 // Adds a scar to this room.
    void        add_mob_spawn(const std::string &id);                   // Adds a Mobile or List to the mobile spawn list.
    void        clear_link_tag(uint8_t id, LinkTag the_tag);            // Clears a tag on this Room's link.
    void        clear_link_tag(Direction dir, LinkTag the_tag);         // As above, but with a Direction enum.
    void        clear_meta(const std::string &key);                     // Clears a metatag from a Room. Use with caution!
    void        clear_tag(RoomTag the_tag);                             // Clears a tag on this Room.
    bool        dangerous_link(Direction dir);                          // Checks if a room link is dangerous (e.g. a sky link).
    bool        dangerous_link(uint8_t dir);                            // As above, but using an integer instead of an enum.
    void        deactivate();                                           // This Room was previously active, and has now become inactive.
    void        decay_scars();                                          // Reduces the intensity of any room scars present.
    std::string desc() const;                                           // Returns the Room's description.
    std::string door_name(Direction dir) const;                         // Returns the name of a door in the specified direction.
    std::string door_name(uint8_t dir) const;                           // As above, but for non-enum integer directions.
    bool        fake_link(Direction dir) const;                         // Checks if a room link is fake (e.g. to FALSE_ROOM or UNFINISHED).
    bool        fake_link(uint8_t dir) const;                           // As above, but takes an integer link instead of an enum.
    size_t      has_campfire() const;                                   // Checks if this room has a campfire, and if so, returns the vector position.
    uint32_t    id() const;                                             // Retrieves the unique hashed ID of this Room.
    const std::shared_ptr<Inventory>    inv() const;                    // Returns a pointer to the Room's Inventory.
    bool        key_can_unlock(std::shared_ptr<Item> key, Direction dir);   // Checks if a key can unlock a door in the specified direction.
    int         light() const;                                          // Gets the light level of this Room, adjusted by dynamic lights, and optionally including darkvision etc.
    uint32_t    link(Direction dir) const;                              // Retrieves a Room link in the specified direction.
    uint32_t    link(uint8_t dir) const;                                // As above, but using an integer.
    bool        link_tag(uint8_t id, LinkTag the_tag) const;            // Checks if a tag is set on this Room's link.
    bool        link_tag(Direction dir, LinkTag the_tag) const;         // As above, but with a Direction enum.
    void        load(std::shared_ptr<SQLite::Database> save_db);        // Loads the Room and anything it contains.
    std::string meta(const std::string &key, bool spaces = true) const; // Retrieves Room metadata.
    std::map<std::string, std::string>* meta_raw();                     // Accesses the metadata map directly. Use with caution!
    std::string name(bool short_name = false) const;                    // Returns the Room's full or short name.
    void        respawn_mobs(bool ignore_timer = false);                // Respawn Mobiles in this Room, if possible.
    void        save(std::shared_ptr<SQLite::Database> save_db);        // Saves the Room and anything it contains.
    std::string scar_desc() const;                                      // Returns the description of any room scars present.
    void        set_base_light(int new_light);                          // Sets this Room's base light level.
    void        set_desc(const std::string &new_desc);                  // Sets this Room's description.
    void        set_link(Direction dir, const std::string &room_id);    // Sets a link to another Room.
    void        set_link(Direction dir, uint32_t room_id);              // As above, but with an already-hashed Room ID.
    void        set_link_tag(uint8_t id, LinkTag the_tag);              // Sets a tag on this Room's link.
    void        set_link_tag(Direction dir, LinkTag the_tag);           // As above, but with a Direction enum.
    void        set_meta(const std::string &key, std::string value);    // Adds Room metadata.
    void        set_name(const std::string &new_name, const std::string &new_short_name);   // Sets the long and short name of this room.
    void        set_security(Security sec);                             // Sets the security level of this Room.
    void        set_tag(RoomTag the_tag);                               // Sets a tag on this Room.
    bool        tag(RoomTag the_tag) const;                             // Checks if a tag is set on this Room.
    int         temperature(uint32_t flags = 0) const;                  // Returns the room's current temperature level.

private:
    static const int    RESPAWN_INTERVAL;                   // The minimum respawn time, in seconds, for Mobiles.
    static const std::vector<std::vector<std::string>>  ROOM_SCAR_DESCS;    // The descriptions for different types of room scars.
    static const int    SEASON_BASE_TEMPERATURE_AUTUMN;     // The base temperature for the autumn season.
    static const int    SEASON_BASE_TEMPERATURE_SPRING;     // The base temperature for the spring season.
    static const int    SEASON_BASE_TEMPERATURE_SUMMER;     // The base temperature for the summer season.
    static const int    SEASON_BASE_TEMPERATURE_WINTER;     // The base temperature for the winter season.
    static const int    WEATHER_TEMPERATURE_MOD_BLIZZARD;   // The temperature modification for blizzard weather.
    static const int    WEATHER_TEMPERATURE_MOD_CLEAR;      // The temperature modification for clear weather.
    static const int    WEATHER_TEMPERATURE_MOD_FAIR;       // The temperature modification for fair weather.
    static const int    WEATHER_TEMPERATURE_MOD_FOG;        // The temperature modification for fog weather.
    static const int    WEATHER_TEMPERATURE_MOD_LIGHTSNOW;  // The temperature modification for light snow weather.
    static const int    WEATHER_TEMPERATURE_MOD_OVERCAST;   // The temperature modification for overcast weather.
    static const int    WEATHER_TEMPERATURE_MOD_RAIN;       // The temperature modification for rain weather.
    static const int    WEATHER_TEMPERATURE_MOD_SLEET;      // The temperature modification for sleet weather.
    static const int    WEATHER_TEMPERATURE_MOD_STORMY;     // The temperature modification for stormy weather.
    static const int    WEATHER_TIME_MOD_DAWN;              // The temperature modification for dawn.
    static const int    WEATHER_TIME_MOD_DUSK;              // The temperature modification for dusk.
    static const int    WEATHER_TIME_MOD_MIDNIGHT;          // The temperature modification for midnight.
    static const int    WEATHER_TIME_MOD_MORNING;           // The temperature modification for morning.
    static const int    WEATHER_TIME_MOD_NIGHT;             // The temperature modification for night.
    static const int    WEATHER_TIME_MOD_NOON;              // The temperature modification for noon.
    static const int    WEATHER_TIME_MOD_SUNRISE;           // The temperature modification for sunrise.
    static const int    WEATHER_TIME_MOD_SUNSET;            // The temperature modification for sunset.

    std::string                 m_desc;                         // The Room's description.
    uint32_t                    m_id;                           // The Room's unique ID, hashed from its YAML name.
    std::shared_ptr<Inventory>  m_inventory;                    // The Room's inventory, for storing dropped items.
    uint32_t                    m_last_spawned_mobs;            // The timer for when this Room last spawned Mobiles.
    uint8_t                     m_light;                        // The default light level of this Room.
    uint32_t                    m_links[ROOM_LINKS_MAX];        // Links to other Rooms.
    std::map<std::string, std::string>  m_metadata;             // The Room's metadata, if any.
    std::string                 m_name;                         // The Room's title.
    std::string                 m_name_short;                   // The Room's short name, for exit listings.
    std::vector<uint8_t>        m_scar_intensity;               // The intensity of the room scars, if any.
    std::vector<ScarType>       m_scar_type;                    // The type of room scars, if any.
    Security                    m_security;                     // The security rating for this Room.
    std::vector<std::string>    m_spawn_mobs;                   // The list of Mobiles to spawn here.
    std::set<RoomTag>           m_tags;                         // Any and all RoomTags on this Room.
    std::set<LinkTag>           m_tags_link[ROOM_LINKS_MAX];    // Any and all LinkTags on this Room's links.
};

}       // namespace greave
#endif  // GREAVE_WORLD_ROOM_H_
