// world/world.hpp -- The World class defines the game world as a whole and handles the passage of time, as well as keeping track of the player's current activities.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Item;                             // defined in world/item.hpp
class List;                             // defined in core/list.hpp
class Mobile;                           // defined in world/mobile.hpp
class Player;                           // defined in world/player.hpp
class Room;                             // defined in world/room.hpp
class TimeWeather;                      // defined in world/time-weather.hpp
enum class DamageType : int8_t;         // defined in world/item.hpp
enum class EquipSlot : uint8_t;         // defined in world/item.hpp
enum class ItemSub : uint16_t;          // defined in world/item.hpp
enum class ItemTag : uint16_t;          // defined in world/item.hpp
enum class ItemType : uint16_t;         // defined in world/item.hpp
enum class LinkTag : uint16_t;          // defined in world/room.hpp
enum class MobileTag : uint16_t;        // defined in world/mobile.hpp
enum class RoomTag : uint16_t;          // defined in world/room.hpp
enum class Security : uint8_t;          // defined in world/room.hpp
namespace SQLite { class Database; }    // defined in 3rdparty/SQLiteCpp/Database.h
struct BodyPart;                        // defined in world/mobile.hpp


class World
{
public:
                    World();                                                    // Constructor, loads the room YAML data.
    std::set<uint32_t>  active_rooms() const;                                   // Retrieve a list of all active rooms.
    void            add_mobile(std::shared_ptr<Mobile> mob);                    // Adds a Mobile to the world.
    std::string     generic_desc(const std::string &id) const;                  // Retrieves a generic description string.
    const std::vector<std::shared_ptr<BodyPart>>& get_anatomy(const std::string &id) const; // Retrieves a copy of the anatomy data for a given species.
    const std::shared_ptr<Item>     get_item(const std::string &item_id, int stack_size = 0) const; // Retrieves a specified Item by ID.
    std::shared_ptr<List>           get_list(const std::string &list_id) const; // Retrieves a specified List by ID.
    const std::shared_ptr<Mobile>   get_mob(const std::string &mob_id) const;   // Retrieves a specified Mobile by ID.
    const std::shared_ptr<Room>     get_room(uint32_t room_id) const;           // Retrieves a specified Room by ID.
    const std::shared_ptr<Room>     get_room(const std::string &room_id) const; // As above, but with a Room ID string.
    bool            item_exists(const std::string &str) const;                  // Checks if a specified item ID exists.
    void            load(std::shared_ptr<SQLite::Database> save_db);            // Loads the World and all things within it.
    const std::shared_ptr<Mobile>   mob(uint32_t vec_pos) const;                // Retrieves a Mobile by vector position.
    unsigned int    mob_count() const;                                          // Returns the number of Mobiles currently active.
    bool            mob_exists(const std::string &str) const;                   // Checks if a specified mobile ID exists.
    void            new_game();                                                 // Sets up for a new game.
    const std::shared_ptr<Player>   player() const;                             // Retrieves a pointer to the Player object.
    void            purge_mobs();                                               // Purges null entries from the active Mobiles. Only call this from the main loop, for safety.
    void            recalc_active_rooms();                                      // Recalculates the list of active rooms.
    void            remove_mobile(std::shared_ptr<Mobile> mob);                 // Removes a Mobile from the world.
    bool            room_active(uint32_t id) const;                             // Checks if a room is currently active.
    bool            room_exists(const std::string &str) const;                  // Checks if a specified room ID exists.
    void            save(std::shared_ptr<SQLite::Database> save_db);            // Saves the World and all things within it.
    const std::shared_ptr<TimeWeather> time_weather() const;                    // Gets a pointer to the TimeWeather object.

private:
    static const std::map<std::string, DamageType>  DAMAGE_TYPE_MAP;    // Lookup table for converting DamageType text names into enums.
    static const std::map<std::string, EquipSlot>   EQUIP_SLOT_MAP;     // Lookup table for converting EquipSlot text names into enums.
    static const std::map<std::string, ItemSub>     ITEM_SUBTYPE_MAP;   // Lookup table for converting ItemSub text names into enums.
    static const std::map<std::string, ItemTag>     ITEM_TAG_MAP;       // Lookup table for converting ItemTag text names into enums.
    static const std::map<std::string, ItemType>    ITEM_TYPE_MAP;      // Lookup table for converting ItemType text names into enums.
    static const std::map<std::string, uint8_t>     LIGHT_LEVEL_MAP;    // Lookup table for converting textual light levels (e.g. "bright") to integer values.
    static const std::map<std::string, LinkTag>     LINK_TAG_MAP;       // Lookup table for converting LinkTag text names into enums.
    static const std::map<std::string, MobileTag>   MOBILE_TAG_MAP;     // Lookup table for converting MobileTag text names into enums.
    static const unsigned int                       ROOM_SCAN_DISTANCE; // The distance to scan for active rooms.
    static const std::map<std::string, RoomTag>     ROOM_TAG_MAP;       // Lookup table for converting RoomTag text names into enums.
    static const std::map<std::string, Security>    SECURITY_MAP;       // Lookup table for converting textual room security (e.g. "anarchy") to enum values.
    static const std::set<std::string>              VALID_YAML_KEYS_AREAS;  // A list of all valid keys in area YAML files.
    static const std::set<std::string>              VALID_YAML_KEYS_ITEMS;  // A list of all valid keys in item YAML files.
    static const std::set<std::string>              VALID_YAML_KEYS_MOBS;   // A list of all valid keys in mobile YAML files.

    std::set<uint32_t>                              m_active_rooms;     // Rooms relatively close to the player, where AI/respawning/etc. will be active.
    std::map<std::string, std::vector<std::shared_ptr<BodyPart>>>   m_anatomy_pool; // The anatomy pool, containing body part data for Mobiles.
    std::map<std::string, std::string>              m_generic_descs;    // Generic descriptions for items and rooms, where multiple share a description.
    std::map<uint32_t, std::shared_ptr<Item>>       m_item_pool;        // All the Item templates in the game.
    std::map<std::string, std::shared_ptr<List>>    m_list_pool;        // List data from lists.yml
    std::map<uint32_t, std::string>                 m_mob_gear;         // Equipment lists for gearing up Mobiles.
    std::map<uint32_t, std::shared_ptr<Mobile>>     m_mob_pool;         // All the Mobile templates in the game.
    std::vector<std::shared_ptr<Mobile>>            m_mobiles;          // All the Mobiles currently active in the game.
    std::shared_ptr<Player>                         m_player;           // The player character.
    std::map<uint32_t, std::shared_ptr<Room>>       m_room_pool;        // All the Room templates in the game.
    std::shared_ptr<TimeWeather>                    m_time_weather;     // The World's TimeWeather object, for tracking... well, the time and weather.

    void    active_room_scan(uint32_t target, uint32_t depth);  // Attempts to scan a room for the active rooms list. Only for internal use with recalc_active_rooms().
    void    load_anatomy_pool();    // Loads the anatomy YAML data into memory.
    void    load_generic_descs();   // Loads the generic descriptions YAML data into memory.
    void    load_item_pool();       // Loads the Item YAML data into memory.
    void    load_lists();           // Loads the List YAML data into memory.
    void    load_mob_pool();        // Loads the Mobile YAML data into memory.
    void    load_room_pool();       // Loads the Room YAML data into memory.
};
