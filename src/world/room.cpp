// world/room.cpp -- The Room class, which defines a single area in the game world that the player can visit.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/yaml-cpp/yaml.h"
#include "core/core.hpp"
#include "core/filex.hpp"
#include "core/message.hpp"
#include "core/strx.hpp"
#include "world/room.hpp"

// Lookup table for converting textual light levels (e.g. "bright") to integer values.
const std::map<std::string, uint8_t>    Room::LIGHT_LEVEL_MAP = { { "bright", 7 }, { "dim", 5 }, { "wilderness", 5 }, { "dark", 3 }, { "none", 0 } };

// Lookup table for converting LinkTag text names into enums.
const std::map<std::string, Room::LinkTag>  Room::LINK_TAG_MAP = { { "doormetal", Room::LinkTag::DoorMetal }, { "lockable", Room::LinkTag::Lockable },
    { "locked", Room::LinkTag::Locked }, { "open", Room::LinkTag::Open }, { "openable", Room::LinkTag::Openable }, { "permalock", Room::LinkTag::Permalock },
    { "window", Room::LinkTag::Window } };

// Lookup table for converting RoomTag text names into enums.
const std::map<std::string, Room::RoomTag>  Room::ROOM_TAG_MAP = { { "indoors", Room::RoomTag::Indoors }, { "noexplorecredit", Room::RoomTag::NoExploreCredit },
    { "private", Room::RoomTag::Private }, { "underground", Room::RoomTag::Underground } };

// Lookup table for converting textual room security (e.g. "anarchy") to enum values.
const std::map<std::string, Room::Security> Room::SECURITY_MAP = { { "anarchy", Security::ANARCHY }, { "losec", Security::LOSEC }, { "hisec", Security::HISEC },
    { "sanctuary", Security::SANCTUARY }, { "inaccessible", Security::INACCESSIBLE } };

std::map<uint32_t, std::shared_ptr<Room>>   Room::s_room_pool;  // All the Rooms in the game.


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

// Loads the Room YAML data into memory.
void Room::load_room_pool()
{
    const std::vector<std::string> area_files = FileX::files_in_dir("data/areas");
    for (auto area_file : area_files)
    {
        const YAML::Node yaml_rooms = YAML::LoadFile("data/areas/" + area_file);
        for (auto room : yaml_rooms)
        {
            const YAML::Node room_data = room.second;

            // Create a new Room object, and set its unique ID.
            const std::string room_id = room.first.as<std::string>();
            const auto new_room(std::make_shared<Room>(room_id));

            // Check to make sure there are no hash collisions.
            if (s_room_pool.find(new_room->id()) != s_room_pool.end()) throw std::runtime_error("Room ID hash conflict: " + room_id);

            // The Room's long and short names.
            if (!room_data["name"] || room_data["name"].size() < 2) core()->message("{r}Missing or invalid room name(s): " + room_id);
            else new_room->set_name(room_data["name"][0].as<std::string>(), room_data["name"][1].as<std::string>());

            // The Room's description.
            if (!room_data["desc"]) core()->message("{r}Missing room description: " + room_id);
            else new_room->set_desc(room_data["desc"].as<std::string>());

            // Links to other Rooms.
            if (room_data["exits"])
            {
                for (unsigned int e = 0; e < ROOM_LINKS_MAX; e++)
                {
                    const Direction dir = static_cast<Direction>(e);
                    const std::string dir_str = StrX::dir_to_name(dir);
                    if (room_data["exits"][dir_str]) new_room->set_link(dir, room_data["exits"][dir_str].as<std::string>());
                }
            }

            // The light level of the Room.
            if (!room_data["light"]) core()->message("{r}Missing room light level: " + room_id);
            else
            {
                const std::string light_str = room_data["light"].as<std::string>();
                auto level_it = LIGHT_LEVEL_MAP.find(light_str);
                if (level_it == LIGHT_LEVEL_MAP.end()) core()->message("{r}Invalid light level value: " + room_id);
                else new_room->set_base_light(level_it->second);
            }

            // The security level of this Room.
            if (!room_data["security"]) core()->message("{r}Missing room security level: " + room_id);
            else
            {
                const std::string sec_str = room_data["security"].as<std::string>();
                auto sec_it = SECURITY_MAP.find(sec_str);
                if (sec_it == SECURITY_MAP.end()) core()->message("{r}Invalid security level value: " + room_id);
                else new_room->set_security(sec_it->second);
            }

            // Room tags, if any.
            if (room_data["tags"])
            {
                if (!room_data["tags"].IsSequence()) core()->message("{r}Malformed room tags: " + room_id);
                else for (auto tag : room_data["tags"])
                {
                    const std::string tag_str = StrX::str_tolower(tag.as<std::string>());
                    bool directional_tag = false;
                    int dt_int = 0, dt_offset = 0;

                    if (tag_str.size() > 9)
                    {
                        if (tag_str.substr(0, 9) == "northeast")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::NORTHEAST);
                            dt_offset = 9;
                        }
                        else if (tag_str.substr(0, 9) == "northwest")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::NORTHWEST);
                            dt_offset = 9;
                        }
                        else if (tag_str.substr(0, 9) == "southeast")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::SOUTHEAST);
                            dt_offset = 9;
                        }
                        else if (tag_str.substr(0, 9) == "southwest")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::SOUTHWEST);
                            dt_offset = 9;
                        }
                    }
                    if (tag_str.size() > 5 && !directional_tag)
                    {
                        if (tag_str.substr(0, 5) == "north")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::NORTH);
                            dt_offset = 5;
                        }
                        else if (tag_str.substr(0, 5) == "south")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::SOUTH);
                            dt_offset = 5;
                        }
                    }
                    if (tag_str.size() > 4 && !directional_tag)
                    {
                        if (tag_str.substr(0, 4) == "east")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::EAST);
                            dt_offset = 4;
                        }
                        else if (tag_str.substr(0, 4) == "west")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::WEST);
                            dt_offset = 4;
                        }
                        else if (tag_str.substr(0, 4) == "down")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::DOWN);
                            dt_offset = 4;
                        }
                    }
                    if (tag_str.size() > 2 && !directional_tag)
                    {
                        if (tag_str.substr(0, 2) == "up")
                        {
                            directional_tag = true;
                            dt_int = static_cast<unsigned int>(Direction::UP);
                            dt_offset = 2;
                        }
                    }

                    if (!directional_tag)
                    {
                        const auto tag_it = ROOM_TAG_MAP.find(tag_str);
                        if (tag_it == ROOM_TAG_MAP.end()) core()->message("{r}Unrecognized room tag (" + tag_str + "): " + room_id);
                        else
                        {
                            const RoomTag rt = tag_it->second;
                            new_room->set_tag(rt);
                        }
                    }
                    else
                    {
                        const std::string dtag_str = tag_str.substr(dt_offset);
                        const auto dtag_it = LINK_TAG_MAP.find(dtag_str);
                        if (dtag_it == LINK_TAG_MAP.end()) core()->message("{r}Unrecognized link tag (" + dtag_str + "): " + room_id);
                        else
                        {
                            const LinkTag lt = dtag_it->second;
                            switch (lt)
                            {
                                case LinkTag::Lockable:
                                case LinkTag::Window:
                                    new_room->set_link_tag(dt_int, LinkTag::Openable);
                                    break;
                                case LinkTag::Locked:
                                    new_room->set_link_tag(dt_int, LinkTag::Lockable);
                                    new_room->set_link_tag(dt_int, LinkTag::Openable);
                                    break;
                                case LinkTag::Open:
                                    new_room->set_link_tag(dt_int, LinkTag::Openable);
                                    break;
                                default: break;
                            }
                            new_room->set_link_tag(dt_int, lt);
                        }
                    }
                }
            }

            // Add the new Room to the room pool.
            s_room_pool.insert(std::make_pair(new_room->id(), new_room));
        }
    }
}

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
