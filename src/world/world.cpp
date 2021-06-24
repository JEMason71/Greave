// world/world.cpp -- The World class defines the game world as a whole and handles the passage of time, as well as keeping track of the player's current activities.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "3rdparty/yaml-cpp/yaml.h"
#include "core/core.hpp"
#include "core/filex.hpp"
#include "core/message.hpp"
#include "core/strx.hpp"
#include "world/player.hpp"
#include "world/room.hpp"
#include "world/world.hpp"


// Lookup table for converting textual light levels (e.g. "bright") to integer values.
const std::map<std::string, uint8_t>    World::LIGHT_LEVEL_MAP = { { "bright", 7 }, { "dim", 5 }, { "wilderness", 5 }, { "dark", 3 }, { "none", 0 } };

// Lookup table for converting LinkTag text names into enums.
const std::map<std::string, LinkTag>    World::LINK_TAG_MAP = { { "doormetal", LinkTag::DoorMetal }, { "hidden", LinkTag::Hidden }, { "lockable", LinkTag::Lockable },
    { "locked", LinkTag::Locked }, { "open", LinkTag::Open }, { "openable", LinkTag::Openable }, { "permalock", LinkTag::Permalock }, { "window", LinkTag::Window } };

// Lookup table for converting RoomTag text names into enums.
const std::map<std::string, RoomTag>    World::ROOM_TAG_MAP = { { "indoors", RoomTag::Indoors }, { "maze", RoomTag::Maze }, { "noexplorecredit", RoomTag::NoExploreCredit },
    { "private", RoomTag::Private }, { "underground", RoomTag::Underground } };

// Lookup table for converting textual room security (e.g. "anarchy") to enum values.
const std::map<std::string, Security>   World::SECURITY_MAP = { { "anarchy", Security::ANARCHY }, { "losec", Security::LOSEC }, { "hisec", Security::HISEC },
    { "sanctuary", Security::SANCTUARY }, { "inaccessible", Security::INACCESSIBLE } };


// Constructor, loads the room YAML data.
World::World()
{
    load_room_pool();
    m_player = std::make_shared<Player>();
}

// Retrieves a specified Room by ID.
const std::shared_ptr<Room> World::get_room(uint32_t room_id) const
{
    const auto it = m_room_pool.find(room_id);
    if (it == m_room_pool.end()) throw std::runtime_error("Invalid room ID requested: " + std::to_string(room_id));
    return it->second;
}

// As above, but with a Room ID string.
const std::shared_ptr<Room> World::get_room(const std::string &room_id) const
{
    if (!room_id.size()) throw std::runtime_error("Blank room ID requested.");
    else return get_room(StrX::hash(room_id));
}

// Loads the Room YAML data into memory.
void World::load_room_pool()
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
            if (m_room_pool.find(new_room->id()) != m_room_pool.end()) throw std::runtime_error("Room ID hash conflict: " + room_id);

            // The Room's long and short names.
            if (!room_data["name"] || room_data["name"].size() < 2) core()->message("{r}Missing or invalid room name(s): " + room_id);
            else new_room->set_name(room_data["name"][0].as<std::string>(), room_data["name"][1].as<std::string>());

            // The Room's description.
            if (!room_data["desc"]) core()->message("{r}Missing room description: " + room_id);
            else new_room->set_desc(room_data["desc"].as<std::string>());

            // Links to other Rooms.
            if (room_data["exits"])
            {
                for (unsigned int e = 0; e < Room::ROOM_LINKS_MAX; e++)
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
            m_room_pool.insert(std::make_pair(new_room->id(), new_room));
        }
    }
}

// Retrieves a pointer to the Player object.
const std::shared_ptr<Mobile> World::player() const { return m_player; }

// Saves the World and all things within it.
void World::save(std::shared_ptr<SQLite::Database> save_db)
{
    save_db->exec(Room::SQL_ROOM_POOL);
    save_db->exec(Player::SQL_PLAYER);
    save_db->exec(Mobile::SQL_MOBILES);
    save_db->exec(MessageLog::SQL_MSGLOG);

    for (auto room : m_room_pool)
        room.second->save(save_db);

    m_player->save(save_db);
    core()->messagelog()->save(save_db);
}

// Loads the World and all things within it.
void World::load(std::shared_ptr<SQLite::Database> save_db)
{
    for (auto room : m_room_pool)
        room.second->load(save_db);
    
    m_player->load(save_db, 0);
    core()->messagelog()->load(save_db);
}
