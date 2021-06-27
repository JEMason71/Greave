// world/world.hpp -- The World class defines the game world as a whole and handles the passage of time, as well as keeping track of the player's current activities.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Mobile;                   // defined in world/mobile.hpp
class Player;                   // defined in world/player.hpp
class Room;                     // defined in world/room.hpp
enum class LinkTag : uint16_t;  // defined in world/room.hpp
enum class RoomTag : uint16_t;  // defined in world/room.hpp
enum class Security : uint8_t;  // defined in world/room.hpp
namespace SQLite { class Database; }    // defined in 3rdparty/SQLiteCpp/Database.h


class World
{
public:
            World();    // Constructor, loads the room YAML data.
    std::string generic_desc(const std::string &id) const;              // Retrieves a generic description string.
    const std::shared_ptr<Room> get_room(uint32_t room_id) const;       // Retrieves a specified Room by ID.
    const std::shared_ptr<Room> get_room(const std::string &room_id) const; // As above, but with a Room ID string.
    void    load(std::shared_ptr<SQLite::Database> save_db);            // Loads the World and all things within it.
    const std::shared_ptr<Mobile>   player() const;                     // Retrieves a pointer to the Player object.
    bool    room_exists(const std::string &str) const;                  // Checks if a specified room ID exists.
    void    save(std::shared_ptr<SQLite::Database> save_db);            // Saves the World and all things within it.

private:
    static const std::map<std::string, uint8_t>     LIGHT_LEVEL_MAP;    // Lookup table for converting textual light levels (e.g. "bright") to integer values.
    static const std::map<std::string, LinkTag>     LINK_TAG_MAP;       // Lookup table for converting LinkTag text names into enums.
    static const std::map<std::string, RoomTag>     ROOM_TAG_MAP;       // Lookup table for converting RoomTag text names into enums.
    static const std::map<std::string, Security>    SECURITY_MAP;       // Lookup table for converting textual room security (e.g. "anarchy") to enum values.

    std::map<std::string, std::string>          m_generic_descs;    // Generic descriptions for items and rooms, where multiple share a description.
    std::shared_ptr<Player>                     m_player;           // The player character.
    std::map<uint32_t, std::shared_ptr<Room>>   m_room_pool;        // All the Rooms in the game.

    void    load_generic_descs();   // Loads the generic descriptions YAML data into memory.
    void    load_room_pool();       // Loads the Room YAML data into memory.
};
