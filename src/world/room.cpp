// world/room.cpp -- The Room class, which defines a single area in the game world that the player can visit.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/yaml-cpp/yaml.h"
#include "core/core.hpp"
#include "core/filex.hpp"
#include "core/message.hpp"
#include "core/strx.hpp"
#include "world/room.hpp"


Room::Room(std::string new_id) : m_light(0), m_security(Security::ANARCHY)
{
    if (new_id.size()) m_id = StrX::hash(new_id);
    else m_id = 0;

    for (unsigned int e = 0; e < ROOM_LINKS_MAX; e++)
        m_links[e] = 0;
}

// Clears a tag on this Room.
void Room::clear_link_tag(unsigned char id, LinkTag the_tag)
{
    if (id >= ROOM_LINKS_MAX) throw std::runtime_error("Invalid direction specified when clearing room link tag.");
	if (!(m_tags_link[id].count(the_tag) > 0)) return;
	m_tags_link[id].erase(the_tag);
}

// As above, but with a Direction enum.
void Room::clear_link_tag(Direction dir, LinkTag the_tag) { clear_link_tag(static_cast<uint8_t>(dir), the_tag); }

// Clears a tag on this Room.
void Room::clear_tag(RoomTag the_tag)
{
    if (!(m_tags.count(the_tag) > 0)) return;
    m_tags.erase(the_tag);
}

// Returns the Room's description.
std::string Room::desc() const { return m_desc; }

// Retrieves the unique hashed ID of this Room.
uint32_t Room::id() const { return m_id; }

// Returns the Room's full or short name.
std::string Room::name(bool short_name) const { return (short_name ? m_name_short : m_name); }

// Sets this Room's base light level.
void Room::set_base_light(uint8_t new_light) { m_light = new_light; }

// Sets this Room's description.
void Room::set_desc(const std::string &new_desc) { m_desc = new_desc; }

// Sets a link to another Room.
void Room::set_link(Direction dir, const std::string &room_id) { set_link(dir, room_id.size() ? StrX::hash(room_id) : 0); }

// As above, but with an already-hashed Room ID.
void Room::set_link(Direction dir, uint32_t room_id)
{
    const int dir_int = static_cast<int>(dir);
    if (dir_int < 0 || static_cast<unsigned int>(dir) >= ROOM_LINKS_MAX) throw std::runtime_error("Invalid direction specified when setting room link.");
    m_links[dir_int] = room_id;
}

// Sets a tag on this Room's link.
void Room::set_link_tag(unsigned char id, LinkTag the_tag)
{
    if (id >= ROOM_LINKS_MAX) throw std::runtime_error("Invalid direction specified when setting room link tag.");
    if (m_tags_link[id].count(the_tag) > 0) return;
    m_tags_link[id].insert(the_tag);
}

// As above, but with a Direction enum.
void Room::set_link_tag(Direction dir, LinkTag the_tag) { set_link_tag(static_cast<uint8_t>(dir), the_tag); }

// Checks if a tag is set on this Room's link.
bool Room::link_tag(unsigned char id, LinkTag the_tag) const
{
    if (id >= ROOM_LINKS_MAX) throw std::runtime_error("Invalid direction specified when checking room link tag.");
    if (the_tag == LinkTag::Lockable || the_tag == LinkTag::Openable || the_tag == LinkTag::Locked || the_tag == LinkTag::Permalock)
    {
        if (m_tags_link[id].count(LinkTag::Permalock) > 0) return true; // If checking for Lockable, Openable or Locked, also check for Permalock.
        if (m_links[id] == FALSE_ROOM) return true; // Links to FALSE_ROOM are always considered to be permalocked.
    }
    return (m_tags_link[id].count(the_tag) > 0);
}

// Sets the long and short name of this room.
void Room::set_name(const std::string &new_name, const std::string &new_short_name)
{
    m_name = new_name;
    m_name_short = new_short_name;
}

// Sets the security level of this Room.
void Room::set_security(Security sec) { m_security = sec; }

// Sets a tag on this Room.
void Room::set_tag(RoomTag the_tag)
{
    if (m_tags.count(the_tag) > 0) return;
    m_tags.insert(the_tag);
}

// Checks if a tag is set on this Room.
bool Room::tag(RoomTag the_tag) const { return (m_tags.count(the_tag) > 0); }
