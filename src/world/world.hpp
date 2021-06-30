// world/world.hpp -- The World class defines the game world as a whole and handles the passage of time, as well as keeping track of the player's current activities.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Item;                             // defined in world/item.hpp
class Mobile;                           // defined in world/mobile.hpp
class Player;                           // defined in world/player.hpp
class Room;                             // defined in world/room.hpp
class TimeWeather;                      // defined in world/time-weather.hpp
enum class EquipSlot : uint8_t;         // defined in world/item.hpp
enum class ItemSub : uint16_t;          // defined in world/item.hpp
enum class ItemTag : uint16_t;          // defined in world/item.hpp
enum class ItemType : uint16_t;         // defined in world/item.hpp
enum class LinkTag : uint16_t;          // defined in world/room.hpp
enum class RoomTag : uint16_t;          // defined in world/room.hpp
enum class Security : uint8_t;          // defined in world/room.hpp
namespace SQLite { class Database; }    // defined in 3rdparty/SQLiteCpp/Database.h


class World
{
public:
                    World();                                                    // Constructor, loads the room YAML data.
    void            add_mobile(std::shared_ptr<Mobile> mob);                    // Adds a Mobile to the world.
    std::string     generic_desc(const std::string &id) const;                  // Retrieves a generic description string.
    const std::shared_ptr<Item>     get_item(const std::string &item_id) const; // Retrieves a specified Item by ID.
    const std::shared_ptr<Mobile>   get_mob(const std::string &mob_id) const;   // Retrieves a specified Mobile by ID.
    const std::shared_ptr<Room>     get_room(uint32_t room_id) const;           // Retrieves a specified Room by ID.
    const std::shared_ptr<Room>     get_room(const std::string &room_id) const; // As above, but with a Room ID string.
    bool            item_exists(const std::string &str) const;                  // Checks if a specified item ID exists.
    void            load(std::shared_ptr<SQLite::Database> save_db);            // Loads the World and all things within it.
    const std::shared_ptr<Mobile>   mob(uint32_t vec_pos) const;                // Retrieves a Mobile by vector position.
    unsigned int    mob_count() const;                                          // Returns the number of Mobiles currently active.
    void            new_game();                                                 // Sets up for a new game.
    const std::shared_ptr<Player>   player() const;                             // Retrieves a pointer to the Player object.
    void            purge_mobs();                                               // Purges null entries from the active Mobiles. Only call this from the main loop, for safety.
    void            remove_mobile(std::shared_ptr<Mobile> mob);                 // Removes a Mobile from the world.
    bool            room_exists(const std::string &str) const;                  // Checks if a specified room ID exists.
    void            save(std::shared_ptr<SQLite::Database> save_db);            // Saves the World and all things within it.
    const std::shared_ptr<TimeWeather> time_weather() const;                    // Gets a pointer to the TimeWeather object.

private:
    static const std::map<std::string, EquipSlot>   EQUIP_SLOT_MAP;     // Lookup table for converting EquipSlot text names into enums.
    static const std::map<std::string, ItemSub>     ITEM_SUBTYPE_MAP;   // Lookup table for converting ItemSub text names into enums.
    static const std::map<std::string, ItemTag>     ITEM_TAG_MAP;       // Lookup table for converting ItemTag text names into enums.
    static const std::map<std::string, ItemType>    ITEM_TYPE_MAP;      // Lookup table for converting ItemType text names into enums.
    static const std::map<std::string, uint8_t>     LIGHT_LEVEL_MAP;    // Lookup table for converting textual light levels (e.g. "bright") to integer values.
    static const std::map<std::string, LinkTag>     LINK_TAG_MAP;       // Lookup table for converting LinkTag text names into enums.
    static const std::map<std::string, RoomTag>     ROOM_TAG_MAP;       // Lookup table for converting RoomTag text names into enums.
    static const std::map<std::string, Security>    SECURITY_MAP;       // Lookup table for converting textual room security (e.g. "anarchy") to enum values.

    std::map<std::string, std::string>          m_generic_descs;    // Generic descriptions for items and rooms, where multiple share a description.
    std::map<uint32_t, std::shared_ptr<Item>>   m_item_pool;        // All the Item templates in the game.
    std::map<uint32_t, std::shared_ptr<Mobile>> m_mob_pool;         // All the Mobile templates in the game.
    std::vector<std::shared_ptr<Mobile>>        m_mobiles;          // All the Mobiles currently active in the game.
    std::shared_ptr<Player>                     m_player;           // The player character.
    std::map<uint32_t, std::shared_ptr<Room>>   m_room_pool;        // All the Room templates in the game.
    std::shared_ptr<TimeWeather>                m_time_weather;     // The World's TimeWeather object, for tracking... well, the time and weather.

    void    load_generic_descs();   // Loads the generic descriptions YAML data into memory.
    void    load_item_pool();       // Loads the Item YAML data into memory.
    void    load_mob_pool();        // Loads the Mobile YAML data into memory.
    void    load_room_pool();       // Loads the Room YAML data into memory.
};
