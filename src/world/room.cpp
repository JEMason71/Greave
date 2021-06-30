// world/room.cpp -- The Room class, which defines a single area in the game world that the player can visit.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.hpp"
#include "core/filex.hpp"
#include "core/message.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/mobile.hpp"
#include "world/room.hpp"
#include "world/time-weather.hpp"
#include "world/world.hpp"


const uint32_t  Room::BLOCKED =         538012167;  // Hashed value for BLOCKED, which is used to mark exits as impassible.
const uint32_t  Room::FALSE_ROOM =      3399618268; // Hashed value for FALSE_ROOM, which is used to make 'fake' impassible room exits.
const uint8_t   Room::LIGHT_VISIBLE =   3;          // Any light level below this is considered too dark to see.
const uint32_t  Room::UNFINISHED =      1909878064; // Hashed value for UNFINISHED, which is used to mark room exits as unfinished and to be completed later.

// The SQL table construction string for the saved rooms.
const std::string   Room::SQL_ROOMS =   "CREATE TABLE rooms ( sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL, id INTEGER UNIQUE NOT NULL, tags TEXT, link_tags TEXT, "
    "inventory INTEGER UNIQUE )";


Room::Room(std::string new_id) : m_inventory(std::make_shared<Inventory>()), m_light(0), m_security(Security::ANARCHY)
{
    if (new_id.size()) m_id = StrX::hash(new_id);
    else m_id = 0;

    for (unsigned int e = 0; e < ROOM_LINKS_MAX; e++)
        m_links[e] = 0;
}

// Clears a tag on this Room.
void Room::clear_link_tag(uint8_t id, LinkTag the_tag)
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
std::string Room::desc() const
{
    const auto time_weather = core()->world()->time_weather();

    auto process_timeweather_desc = [] (std::string &desc, std::string tag, bool active) {
        const size_t start = desc.find("[" + tag);
        const size_t end = desc.find("]", start);
        if (active)
        {
            const unsigned int insert_start = start + tag.size() + 2;
            const std::string insert = desc.substr(insert_start, end - insert_start);
            desc = desc.substr(0, start) + insert + desc.substr(end + 1);
        }
        else desc = desc.substr(0, start) + desc.substr(end + 1);
    };

    std::string desc = m_desc;
    if (m_desc.size() > 2 && m_desc[0] == '$') desc = core()->world()->generic_desc(m_desc.substr(1));
    const TimeWeather::Season current_season = time_weather->current_season();
    const TimeWeather::TimeOfDay current_tod = time_weather->time_of_day(false);
    while (desc.find("[springsummer:") != std::string::npos)
        process_timeweather_desc(desc, "springsummer", current_season == TimeWeather::Season::SPRING || current_season == TimeWeather::Season::SUMMER);
    while (desc.find("[autumnwinter:") != std::string::npos)
        process_timeweather_desc(desc, "autumnwinter", current_season == TimeWeather::Season::AUTUMN || current_season == TimeWeather::Season::WINTER);
    while (desc.find("[daydawn:") != std::string::npos)
        process_timeweather_desc(desc, "daydawn", current_tod == TimeWeather::TimeOfDay::DAY || current_tod == TimeWeather::TimeOfDay::DAWN);
    while (desc.find("[nightdusk:") != std::string::npos)
        process_timeweather_desc(desc, "nightdusk", current_tod == TimeWeather::TimeOfDay::NIGHT || current_tod == TimeWeather::TimeOfDay::DUSK);

    return desc;
}

// Returns the name of a door in the specified direction.
std::string Room::door_name(Direction dir) const
{
    if (static_cast<uint8_t>(dir) >= ROOM_LINKS_MAX) throw std::runtime_error("Invalid direction specified when checking door name.");
    if (!link_tag(dir, LinkTag::Openable)) return "";
    if (link_tag(dir, LinkTag::Window)) return "window";
    if (link_tag(dir, LinkTag::DoorMetal)) return "metal door";
    return "door";
}

// As above, but for non-enum integer directions.
std::string Room::door_name(uint8_t dir) const { return door_name(static_cast<Direction>(dir)); }

// Checks if a room link is fake (e.g. to FALSE_ROOM or UNFINISHED).
bool Room::fake_link(Direction dir) const
{
    const uint32_t link_id = link(dir);
    if (!link_id || link_id == FALSE_ROOM || link_id == UNFINISHED || link_id == BLOCKED) return true;
    else return false;
}

// Retrieves the unique hashed ID of this Room.
uint32_t Room::id() const { return m_id; }

// Returns a pointer to the Room's Inventory.
const std::shared_ptr<Inventory> Room::inv() const { return m_inventory; }

// Checks if a key can unlock a door in the specified direction.
bool Room::key_can_unlock(std::shared_ptr<Item> key, Direction dir)
{
    // Ignore fake links (FALSE_ROOM, UNFINISHED, BLOCKED, etc.), permalocks, non-lockable exits, and non-key items.
    if (fake_link(dir) || link_tag(dir, LinkTag::Permalock) || !link_tag(dir, LinkTag::Lockable) || key->type() != ItemType::KEY) return false;

    // Get the key's metadata. If none, it can't open anything.
	const std::string key_meta = key->meta("key");
	if (!key_meta.size()) return false;

    const uint32_t link_id = link(dir);
    const std::shared_ptr<Room> dest_room = core()->world()->get_room(link_id);
    const std::vector<std::string> keys = StrX::string_explode(key_meta, ",");
    for (auto key : keys)   // Check any and all 'key' metadata on the key, see if it matches the source or destination room, or is a skeleton key.
    {
        const uint32_t key_hash = StrX::hash(key);
        if (key == "SKELETON" || key_hash == link_id || key_hash == m_id) return true;
    }
    return false;
}

// Gets the light level of this Room, adjusted by dynamic lights, and optionally including darkvision etc.
int Room::light(std::shared_ptr<Mobile> mob) const
{
    int dynamic_light = m_light;
    const auto equ = mob->equ();

    // Check for equipped light sources.
    for (unsigned int i = 0; i < equ->count(); i++)
    {
        const auto item = equ->get(i);
        if (item->type() != ItemType::LIGHT) continue;

        // If a light source's power is brighter than the dynamic light so far, up the dynamic light to its power.
        // It's not *additive*, light sources will never make a bright room brighter, they just bring the light level up to a minimum value.
        if (item->power() > dynamic_light) dynamic_light = item->power();
    }

    // Check for anything glowing on the floor.
    for (unsigned int i = 0; i < m_inventory->count(); i++)
    {
        const auto item = m_inventory->get(i);
        if (item->type() != ItemType::LIGHT) continue;
        if (item->power() > dynamic_light) dynamic_light = item->power();
    }

    return dynamic_light;
}

// Retrieves a Room link in the specified direction.
uint32_t Room::link(Direction dir) const { return link(static_cast<uint8_t>(dir)); }

// As above, but using an integer.
uint32_t Room::link(uint8_t dir) const
{
    if (dir >= ROOM_LINKS_MAX) throw std::runtime_error("Invalid direction specified when checking room links.");
    return m_links[dir];
}

// Checks if a tag is set on this Room's link.
bool Room::link_tag(uint8_t id, LinkTag the_tag) const
{
    if (id >= ROOM_LINKS_MAX) throw std::runtime_error("Invalid direction specified when checking room link tag.");
    if (the_tag == LinkTag::Lockable || the_tag == LinkTag::Openable || the_tag == LinkTag::Locked)
    {
        if (m_tags_link[id].count(LinkTag::Permalock) > 0) return true; // If checking for Lockable, Openable or Locked, also check for Permalock.
        if (m_links[id] == FALSE_ROOM) return true; // Links to FALSE_ROOM are always considered to be permalocked.

        // Special rules check here. Because exits are usually unlocked by default, but may have the LockedByDefault tag, they then require Unlocked
        // to mark them as currently unlocked. To simplify things, we'll just check for the Locked tag externally, and handle this special case here.
        if (the_tag == LinkTag::Locked)
        {
            if (m_tags_link[id].count(LinkTag::Locked) > 0) return true;    // If it's marked as Locked then it's locked, no question.
            if (m_tags_link[id].count(LinkTag::LockedByDefault) > 0)        // And here's the tricky bit.
            {
                if (m_tags_link[id].count(LinkTag::Unlocked) > 0) return false; // If it's marked Unlocked, then we're good.
                else return true;   // If not, then yes, it's locked.
            }
        }
    }
    return (m_tags_link[id].count(the_tag) > 0);
}

// As above, but with a Direction enum.
bool Room::link_tag(Direction dir, LinkTag the_tag) const { return link_tag(static_cast<uint8_t>(dir), the_tag); }

// Returns the Room's full or short name.
std::string Room::name(bool short_name) const { return (short_name ? m_name_short : m_name); }

// Saves the Room and anything it contains.
void Room::save(std::shared_ptr<SQLite::Database> save_db)
{
    const uint32_t inventory_id = m_inventory->save(save_db);

    const std::string tags = StrX::tags_to_string(m_tags);
    std::string link_tags;
    for (unsigned int e = 0; e < ROOM_LINKS_MAX; e++)
    {
        link_tags += StrX::tags_to_string(m_tags_link[e]);
        if (e < ROOM_LINKS_MAX - 1) link_tags += ",";
    }

    if (!tags.size() && link_tags == ",,,,,,,,,") return;

    SQLite::Statement room_query(*save_db, "INSERT INTO rooms (sql_id, id, tags, link_tags, inventory) VALUES (?, ?, ?, ?, ?)");
    room_query.bind(1, core()->sql_unique_id());
    room_query.bind(2, m_id);
    if (tags.size()) room_query.bind(3, tags);
    if (link_tags != ",,,,,,,,,") room_query.bind(4, link_tags);
    if (inventory_id) room_query.bind(5, inventory_id);
    room_query.exec();
}

// Loads the Room and anything it contains.
void Room::load(std::shared_ptr<SQLite::Database> save_db)
{
    uint32_t inventory_id = 0;
    SQLite::Statement query(*save_db, "SELECT * FROM rooms WHERE id = ?");
    query.bind(1, m_id);
    if (query.executeStep())
    {
        if (!query.getColumn("tags").isNull()) StrX::string_to_tags(query.getColumn("tags").getString(), m_tags);
        if (!query.getColumn("link_tags").isNull())
        {
            const std::string link_tags_str = query.getColumn("link_tags").getString();
            std::vector<std::string> split_links = StrX::string_explode(link_tags_str, ",");
            if (split_links.size() != ROOM_LINKS_MAX) throw std::runtime_error("Malformed room link tags data.");
            for (unsigned int e = 0; e < ROOM_LINKS_MAX; e++)
            {
                if (!split_links.at(e).size()) continue;
                std::vector<std::string> split_tags = StrX::string_explode(split_links.at(e), " ");
                for (auto tag : split_tags)
                    m_tags_link[e].insert(static_cast<LinkTag>(StrX::htoi(tag)));
            }
        }
        inventory_id = query.getColumn("inventory").getUInt();
    }
    if (inventory_id) m_inventory->load(save_db, inventory_id);
}

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
void Room::set_link_tag(uint8_t id, LinkTag the_tag)
{
    if (id >= ROOM_LINKS_MAX) throw std::runtime_error("Invalid direction specified when setting room link tag.");
    if (m_tags_link[id].count(the_tag) > 0) return;
    m_tags_link[id].insert(the_tag);
}

// As above, but with a Direction enum.
void Room::set_link_tag(Direction dir, LinkTag the_tag) { set_link_tag(static_cast<uint8_t>(dir), the_tag); }

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
