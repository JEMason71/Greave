// world/room.hpp -- The Room class, which defines a single area in the game world that the player can visit.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


enum class LinkTag : uint16_t {
    // Tags below 10,000 are considered *dynamic tags*. These tags WILL be saved to save files.
    Open = 1,           // A door, gate, hatch, etc. that is currently open.
    Locked = 2,         // A closed door etc. that is currently locked.

    // Tags at 10,000 or above are considered *permanent tags*. These tags WILL NOT be saved to save files.
    Openable = 10000,   // Can be opened and closed; a door, hatch, gate, or similar.
    Lockable = 10001,   // Can be locked or unlocked with a key. Must be openable.
    Permalock = 10002,  // A locked door/etc. which can never be unlocked.
    Hidden = 10003,     // This link is not normally visible.
    DoorMetal = 10004,  // Is the door on this link metal?
    Window = 10005,     // This 'door' is actually a window.
};

enum class RoomTag : uint16_t {
    // Tags below 10,000 are considered *dynamic tags*. These tags WILL be saved to save files.
    Explored = 1,           // The player has visited this room before.

    // Tags at 10,000 or above are considered *permanent tags*. These tags WILL NOT be saved to save files.
    Indoors = 10000,        // Is this room indoors?
    Underground = 10001,    // Is this room underground?
    Private = 10002,        // Entering this room is considered trespassing.
    NoExploreCredit = 10003,    // This room does not count towards the number of 'explored' rooms in the player's stats.
    Maze = 10004,           // This Room is part of a maze, and should not have its exit names labeled.
};

enum class Security : uint8_t { ANARCHY, LOSEC, HISEC, SANCTUARY, INACCESSIBLE };

class Room
{
public:
    static const uint32_t       FALSE_ROOM = 3399618268;        // Hashed value for FALSE_ROOM, which is used to make 'fake' impassible room exits.
    static const uint8_t        LIGHT_VISIBLE = 3;              // Any light level below this is considered too dark to see.
    static const unsigned int   ROOM_LINKS_MAX = 10;            // The maximum amount of exit links from one Room to another.

                Room(std::string new_id = "");                  // Constructor, sets the Room's ID hash.
	void        clear_link_tag(uint8_t id, LinkTag the_tag);    // Clears a tag on this Room's link.
	void        clear_link_tag(Direction dir, LinkTag the_tag); // As above, but with a Direction enum.
    void        clear_tag(RoomTag the_tag);                     // Clears a tag on this Room.
    std::string desc() const;                                   // Returns the Room's description.
    std::string door_name(Direction dir) const;                 // Returns the name of a door in the specified direction.
    uint32_t    id() const;                                     // Retrieves the unique hashed ID of this Room.
    int         light(std::shared_ptr<Mobile> mob = nullptr) const; // Gets the light level of this Room, adjusted by dynamic lights, and optionally including darkvision etc.
    uint32_t    link(Direction dir) const;                      // Retrieves a Room link in the specified direction.
    uint32_t    link(uint8_t dir) const;                        // As above, but using an integer.
	bool        link_tag(uint8_t id, LinkTag the_tag) const;    // Checks if a tag is set on this Room's link.
	bool        link_tag(Direction dir, LinkTag the_tag) const; // As above, but with a Direction enum.
    std::string name(bool short_name = false) const;            // Returns the Room's full or short name.
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
    uint8_t     m_light;        // The default light level of this Room.
    uint32_t    m_links[ROOM_LINKS_MAX];    // Links to other Rooms.
    std::string m_name;         // The Room's title.
    std::string m_name_short;   // The Room's short name, for exit listings.
    Security    m_security;     // The security rating for this Room.
    std::set<RoomTag>   m_tags; // Any and all RoomTags on this Room.
    std::set<LinkTag>   m_tags_link[ROOM_LINKS_MAX];    // Any and all LinkTags on this Room's links.
};
