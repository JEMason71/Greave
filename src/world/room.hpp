// world/room.hpp -- The Room class, which defines a single area in the game world that the player can visit.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


enum class LinkTag : uint16_t {
    // 1-99 -- basic link attributes, pretty core stuff here.
    Openable = 1,       // Can be opened and closed; a door, hatch, gate, or similar.
    Lockable = 2,       // Can be locked or unlocked with a key. Must be openable.
    Permalock = 3,      // As above, but can never be unlocked.

    // 100-199 block -- tags regarding a transient status of the exit, such as open doors.
    Open = 100,         // A door, gate, hatch, etc. that is currently open.
    Locked = 101,       // A closed door etc. that is currently locked.

    // 200-299 block -- tags regarding permanent attributes of doors and locks.
    DoorMetal = 200,    // Is the door on this link metal?
    Window = 201,       // This 'door' is actually a window.
};

enum class RoomTag : uint16_t {
    // 1-99 block -- basic room attributes, pretty core stuff here.
    Indoors = 1,            // Is this room indoors?
    Underground = 2,        // Is this room underground?
    Private = 3,            // Entering this room is considered trespassing.
    NoExploreCredit = 4,    // This room does not count towards the number of 'explored' rooms in the player's stats.
};

enum class Security : uint8_t { ANARCHY, LOSEC, HISEC, SANCTUARY, INACCESSIBLE };

class Room
{
public:
    static const uint32_t FALSE_ROOM = 3399618268;  // Hashed value for FALSE_ROOM, which is used to make 'fake' impassible room exits.
    static const unsigned int ROOM_LINKS_MAX = 10;  // The maximum amount of exit links from one Room to another.

                Room(std::string new_id = "");  // Constructor, sets the Room's ID hash.
	void        clear_link_tag(unsigned char id, LinkTag the_tag);  // Clears a tag on this Room's link.
	void        clear_link_tag(Direction dir, LinkTag the_tag);     // As above, but with a Direction enum.
    void        clear_tag(RoomTag the_tag);     // Clears a tag on this Room.
    std::string desc() const;   // Returns the Room's description.
    uint32_t    id() const;     // Retrieves the unique hashed ID of this Room.
	bool        link_tag(unsigned char id, LinkTag the_tag) const;  // Checks if a tag is set on this Room's link.
	bool        link_tag(Direction dir, LinkTag the_tag) const;     // As above, but with a Direction enum.
    std::string name(bool short_name = false) const;    // Returns the Room's full or short name.
    void        set_base_light(uint8_t new_light);  // Sets this Room's base light level.
    void        set_desc(const std::string &new_desc);  // Sets this Room's description.
    void        set_link(Direction dir, const std::string &room_id);    // Sets a link to another Room.
    void        set_link(Direction dir, uint32_t room_id);  // As above, but with an already-hashed Room ID.
	void        set_link_tag(unsigned char id, LinkTag the_tag);    // Sets a tag on this Room's link.
	void        set_link_tag(Direction dir, LinkTag the_tag);       // As above, but with a Direction enum.
    void        set_name(const std::string &new_name, const std::string &new_short_name);   // Sets the long and short name of this room.
    void        set_security(Security sec); // Sets the security level of this Room.
    void        set_tag(RoomTag the_tag);   // Sets a tag on this Room.
    bool        tag(RoomTag the_tag) const; // Checks if a tag is set on this Room.

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
