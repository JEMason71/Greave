// world/room.hpp -- The Room class, which defines a single area in the game world that the player can visit.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Inventory;    // defined in world/inventory.hpp
class Item;         // defined in world/item.hpp
namespace SQLite { class Database; }    // defined in 3rdparty/SQLiteCpp/Database.h


enum class LinkTag : uint16_t {
    // Tags below 10,000 are considered *dynamic tags*. These tags WILL be saved to save files.
    Open = 1,           // A door, gate, hatch, etc. that is currently open.
    Locked,             // A closed door etc. that is currently locked.
    Unlocked,           // As above, but one that is explicitly unlocked.
    KnownLocked,        // The player is aware that this exit is locked.

    // Tags at 10,000 or above are considered *permanent tags*. These tags WILL NOT be saved to save files.
    Openable = 10000,   // Can be opened and closed; a door, hatch, gate, or similar.
    Lockable,           // Can be locked or unlocked with a key. Must be openable. [CURRENTLY UNUSED]
    LockedByDefault,    // This exit is locked by default (requires the Unlocked tag to be considered unlocked).
    LockWeak,           // Does this door have a particularly weak lock? [CURRENTLY UNUSED]
    LockStrong,         // Does this door have a particularly strong lock? [CURRENTLY UNUSED]
    Permalock,          // A locked door/etc. which can never be unlocked.
    Hidden,             // This link is not normally visible.
    DoorMetal,          // Is the door on this link metal?
    Window,             // This 'door' is actually a window.
    DoorShop,           // This is the door to a shop and should be locked at night. [CURRENTLY UNUSED]
    AutoClose,          // This door will automatically close after the player passes through it. [CURRENTLY UNUSED]
    AutoLock,           // This door will automatically lock after the player passes through it. [CURRENTLY UNUSED]
    LocksWhenClosed,    // When this door is closed, it will automatically lock itself. [CURRENTLY UNUSED]
    DoubleLength,       // This link is twice as long, so will take extra time to traverse. [CURRENTLY UNUSED]
    TripleLength,       // This link is three times as long, so will take extra time to traverse. [CURRENTLY UNUSED]
    Ocean,              // For links that go into the ocean. [CURRENTLY UNUSED]
    Sky,                // This link leads to a virtual sky room, with the actual linked room below. [CURRENTLY UNUSED]
    Sky2,               // As above, but the sky room is stacked two rooms high. [CURRENTLY UNUSED]
    Sky3,               // As above, but three rooms high. [CURRENTLY UNUSED]
    Incline,            // This link also leads upward. [CURRENTLY UNUSED]
    Decline,            // This link also leads downward. [CURRENTLY UNUSED]
    NoMobRoam,          // NPCs should not roam through this exit. [CURRENTLY UNUSED]
    NoBlockExit,        // NPCs should not block the player passing this way, even if they're going to a private room. [CURRENTLY UNUSED]
};

enum class RoomTag : uint16_t {
    // Tags below 10,000 are considered *dynamic tags*. These tags WILL be saved to save files.
    Explored = 1,           // The player has visited this room before.

    // Tags at 10,000 or above are considered *permanent tags*. These tags WILL NOT be saved to save files.
    Indoors = 10000,    // Is this room indoors? [CURRENTLY UNUSED]
    Underground,        // Is this room underground? [CURRENTLY UNUSED]
    Private,            // Entering this room is considered trespassing. [CURRENTLY UNUSED]
    NoExploreCredit,    // This room does not count towards the number of 'explored' rooms in the player's stats. [CURRENTLY UNUSED]
    Maze,               // This Room is part of a maze, and should not have its exit names labeled. [CURRENTLY UNUSED]
    CanSeeOutside,      // Are we indoors, but can see the weather outside? [CURRENTLY UNUSED]
    DigOK,              // The ground here can be dug up easily with a shovel. [CURRENTLY UNUSED]
    Wide,               // Is this room quite spacious? [CURRENTLY UNUSED]
    VeryWide,           // Is this room extremely spacious? [CURRENTLY UNUSED]
    WaterClean,         // There is a source of clean water nearby. [CURRENTLY UNUSED]
    WaterTainted,       // There is a water source nearby, but it is unclean. [CURRENTLY UNUSED]
    WaterSalt,          // There is a source of salt water nearby. [CURRENTLY UNUSED]
    WaterShallow,       // Is this shallow water, the kind that doesn't require swimming? [CURRENTLY UNUSED]
    WaterDeep,          // Is this deep water, which can be drowned in? [CURRENTLY UNUSED]
    Trees,              // There are trees in this area. [CURRENTLY UNUSED]
    ShopBuysContraband, // This room's shop is willing to buy stolen/contraband items. [CURRENTLY UNUSED]
    ShopRespawningOwner,    // This shop's owner NPC respawns or is replaced. [CURRENTLY UNUSED]
    PermaCampfire,      // Treat this room like it always has a campfire burning. [CURRENTLY UNUSED]
    HideCampfireScar,   // This is a bit specific. It's for rooms with PermaCampfire, where we don't want the campfire 'scar' text showing. [CURRENTLY UNUSED]
    SleepOK,            // Environmental noises will affect the player much less here. [CURRENTLY UNUSED]
    Nexus,              // This room contains a nexus transportation system. [CURRENTLY UNUSED]
    RadiationLight,     // This area is lightly irradiated. [CURRENTLY UNUSED]
    SludgePit,          // We got a sinky sludge pit here, guys. [CURRENTLY UNUSED]
    GameSlots,          // This room contains a slot machine minigame. [CURRENTLY UNUSED]
    GamePoker,          // This room contains a video poker minigame. [CURRENTLY UNUSED]
    ChurchAltar,        // This room is a church altar, we can respawn here. [CURRENTLY UNUSED]
    Gross,              // This room is disgusting, and can trigger sanity effects. [CURRENTLY UNUSED]
    Smelly,             // As with the Gross tag, but in this case the effect is triggered by smell, not sight. [CURRENTLY UNUSED]
    HeatedInterior,     // Is this interior area heated? [CURRENTLY UNUSED]
};

enum class Security : uint8_t { ANARCHY, LOW, HIGH, SANCTUARY, INACCESSIBLE };

class Room
{
public:
    static const uint32_t       BLOCKED;                        // Hashed value for BLOCKED, which is used to mark exits as impassible.
    static const uint32_t       FALSE_ROOM;                     // Hashed value for FALSE_ROOM, which is used to make 'fake' impassible room exits.
    static const uint8_t        LIGHT_VISIBLE;                  // Any light level below this is considered too dark to see.
    static const unsigned int   ROOM_LINKS_MAX = 10;            // The maximum amount of exit links from one Room to another.
    static const std::string    SQL_ROOMS;                      // The SQL table construction string for the saved rooms.
    static const uint32_t       UNFINISHED;                     // Hashed value for UNFINISHED, which is used to mark room exits as unfinished and to be completed later.

                Room(std::string new_id = "");                  // Constructor, sets the Room's ID hash.
	void        clear_link_tag(uint8_t id, LinkTag the_tag);    // Clears a tag on this Room's link.
	void        clear_link_tag(Direction dir, LinkTag the_tag); // As above, but with a Direction enum.
    void        clear_tag(RoomTag the_tag);                     // Clears a tag on this Room.
    std::string desc() const;                                   // Returns the Room's description.
    std::string door_name(Direction dir) const;                 // Returns the name of a door in the specified direction.
    std::string door_name(uint8_t dir) const;                   // As above, but for non-enum integer directions.
    bool        fake_link(Direction dir) const;                 // Checks if a room link is fake (e.g. to FALSE_ROOM or UNFINISHED).
    uint32_t    id() const;                                     // Retrieves the unique hashed ID of this Room.
    const std::shared_ptr<Inventory>    inv() const;            // Returns a pointer to the Room's Inventory.
    bool        key_can_unlock(std::shared_ptr<Item> key, Direction dir);   // Checks if a key can unlock a door in the specified direction.
    int         light(std::shared_ptr<Mobile> mob = nullptr) const; // Gets the light level of this Room, adjusted by dynamic lights, and optionally including darkvision etc.
    uint32_t    link(Direction dir) const;                      // Retrieves a Room link in the specified direction.
    uint32_t    link(uint8_t dir) const;                        // As above, but using an integer.
	bool        link_tag(uint8_t id, LinkTag the_tag) const;    // Checks if a tag is set on this Room's link.
	bool        link_tag(Direction dir, LinkTag the_tag) const; // As above, but with a Direction enum.
    void        load(std::shared_ptr<SQLite::Database> save_db);    // Loads the Room and anything it contains.
    std::string name(bool short_name = false) const;            // Returns the Room's full or short name.
    void        save(std::shared_ptr<SQLite::Database> save_db);    // Saves the Room and anything it contains.
    void        set_base_light(uint8_t new_light);              // Sets this Room's base light level.
    void        set_desc(const std::string &new_desc);          // Sets this Room's description.
    void        set_link(Direction dir, const std::string &room_id);    // Sets a link to another Room.
    void        set_link(Direction dir, uint32_t room_id);      // As above, but with an already-hashed Room ID.
	void        set_link_tag(uint8_t id, LinkTag the_tag);      // Sets a tag on this Room's link.
	void        set_link_tag(Direction dir, LinkTag the_tag);   // As above, but with a Direction enum.
    void        set_name(const std::string &new_name, const std::string &new_short_name);   // Sets the long and short name of this room.
    void        set_security(Security sec);                     // Sets the security level of this Room.
    void        set_tag(RoomTag the_tag);                       // Sets a tag on this Room.
    bool        tag(RoomTag the_tag) const;                     // Checks if a tag is set on this Room.

private:
    std::string m_desc;         // The Room's description.
    uint32_t    m_id;           // The Room's unique ID, hashed from its YAML name.
    std::shared_ptr<Inventory>  m_inventory;    // The Room's inventory, for storing dropped items.
    uint8_t     m_light;        // The default light level of this Room.
    uint32_t    m_links[ROOM_LINKS_MAX];    // Links to other Rooms.
    std::string m_name;         // The Room's title.
    std::string m_name_short;   // The Room's short name, for exit listings.
    Security    m_security;     // The security rating for this Room.
    std::set<RoomTag>   m_tags; // Any and all RoomTags on this Room.
    std::set<LinkTag>   m_tags_link[ROOM_LINKS_MAX];    // Any and all LinkTags on this Room's links.
};
