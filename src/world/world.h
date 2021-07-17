// world/world.h -- The World class defines the game world as a whole and handles the passage of time, as well as keeping track of the player's current activities.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_WORLD_WORLD_H_
#define GREAVE_WORLD_WORLD_H_

#include <cstddef>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "3rdparty/SQLiteCpp/Database.h"
#include "core/list.h"
#include "world/player.h"
#include "world/room.h"
#include "world/shop.h"
#include "world/time-weather.h"


namespace greave {

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
    const std::shared_ptr<Shop> get_shop(uint32_t id);                          // Returns a specified shop, or creates a new shop if this ID doesn't yet exist.
    float           get_skill_multiplier(const std::string &skill);             // Retrieves the XP gain multiplier for a specified skill.
    std::string     get_skill_name(const std::string &skill);                   // Retrieves the name of a specified skill.
    bool            item_exists(const std::string &str) const;                  // Checks if a specified item ID exists.
    void            load(std::shared_ptr<SQLite::Database> save_db);            // Loads the World and all things within it.
    void            main_loop_events_post_input();                              // Triggers events that happen during the main loop, just after player input.
    void            main_loop_events_pre_input();                               // Triggers events that happen during the main loop, just before player input.
    size_t          mob_count() const;                                          // Returns the number of Mobiles currently active.
    bool            mob_exists(const std::string &str) const;                   // Checks if a specified mobile ID exists.
    const std::shared_ptr<Mobile>   mob_vec(size_t vec_pos) const;              // Retrieves a Mobile by vector position.
    void            new_game();                                                 // Sets up for a new game.
    const std::shared_ptr<Player>   player() const;                             // Retrieves a pointer to the Player object.
    void            recalc_active_rooms();                                      // Recalculates the list of active rooms.
    void            remove_mobile(size_t id);                                   // Removes a Mobile from the world.
    bool            room_active(uint32_t id) const;                             // Checks if a room is currently active.
    bool            room_exists(const std::string &str) const;                  // Checks if a specified room ID exists.
    void            save(std::shared_ptr<SQLite::Database> save_db);            // Saves the World and all things within it.
    void            starter_equipment(const std::string &list_name);            // Assigns the player starter equipment from a list.
    const std::shared_ptr<TimeWeather> time_weather() const;                    // Gets a pointer to the TimeWeather object.

private:
    struct SkillData
    {
        std::string name;       // The name of this skill.
        float       xp_multi;   // The multiplier applied to the XP gained when using this skill.
    };

    static const std::map<std::string, DamageType>  DAMAGE_TYPE_MAP;        // Lookup table for converting DamageType text names into enums.
    static const std::map<std::string, EquipSlot>   EQUIP_SLOT_MAP;         // Lookup table for converting EquipSlot text names into enums.
    static const std::map<std::string, ItemSub>     ITEM_SUBTYPE_MAP;       // Lookup table for converting ItemSub text names into enums.
    static const std::map<std::string, ItemTag>     ITEM_TAG_MAP;           // Lookup table for converting ItemTag text names into enums.
    static const std::map<std::string, ItemType>    ITEM_TYPE_MAP;          // Lookup table for converting ItemType text names into enums.
    static const std::map<std::string, uint8_t>     LIGHT_LEVEL_MAP;        // Lookup table for converting textual light levels (e.g. "bright") to integer values.
    static const std::map<std::string, LinkTag>     LINK_TAG_MAP;           // Lookup table for converting LinkTag text names into enums.
    static const std::map<std::string, MobileTag>   MOBILE_TAG_MAP;         // Lookup table for converting MobileTag text names into enums.
    static const int                                ROOM_SCAN_DISTANCE;     // The distance to scan for active rooms.
    static const std::map<std::string, RoomTag>     ROOM_TAG_MAP;           // Lookup table for converting RoomTag text names into enums.
    static const std::map<std::string, Security>    SECURITY_MAP;           // Lookup table for converting textual room security (e.g. "anarchy") to enum values.
    static const std::string                        SQL_WORLD;              // The SQL construction table for the World data.
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
    uint32_t                                        m_mob_unique_id;    // The unique ID counter for Mobiles.
    std::vector<std::shared_ptr<Mobile>>            m_mobiles;          // All the Mobiles currently active in the game.
    int                                             m_old_light_level;  // Used to check when the light level changes in the player's room.
    uint32_t                                        m_old_location;     // Also used for light level change checks.
    std::shared_ptr<Player>                         m_player;           // The player character.
    std::map<uint32_t, std::shared_ptr<Room>>       m_room_pool;        // All the Room templates in the game.
    std::map<uint32_t, std::shared_ptr<Shop>>       m_shops;            // Any and all shops in the game.
    std::map<std::string, SkillData>                m_skills;           // The skills the player can use.
    std::shared_ptr<TimeWeather>                    m_time_weather;     // The World's TimeWeather object, for tracking... well, the time and weather.

    void    active_room_scan(uint32_t target, uint32_t depth);  // Attempts to scan a room for the active rooms list. Only for internal use with recalc_active_rooms().
    void    load_anatomy_pool();    // Loads the anatomy YAML data into memory.
    void    load_generic_descs();   // Loads the generic descriptions YAML data into memory.
    void    load_item_pool();       // Loads the Item YAML data into memory.
    void    load_lists();           // Loads the List YAML data into memory.
    void    load_mob_pool();        // Loads the Mobile YAML data into memory.
    void    load_room_pool();       // Loads the Room YAML data into memory.
    void    load_skills();          // Laods the skills YAML data into memory.
};

}       // namespace greave
#endif  // GREAVE_WORLD_WORLD_H_
