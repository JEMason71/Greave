// world/room.cpp -- The Room class, which defines a single area in the game world that the player can visit.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.hpp"
#include "core/filex.hpp"
#include "core/list.hpp"
#include "core/message.hpp"
#include "core/random.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/player.hpp"
#include "world/room.hpp"
#include "world/time-weather.hpp"
#include "world/world.hpp"


const uint32_t  Room::BLOCKED =             538012167;  // Hashed value for BLOCKED, which is used to mark exits as impassible.
const uint32_t  Room::FALSE_ROOM =          3399618268; // Hashed value for FALSE_ROOM, which is used to make 'fake' impassible room exits.
const uint8_t   Room::LIGHT_VISIBLE =       3;          // Any light level below this is considered too dark to see.
const uint32_t  Room::UNFINISHED =          1909878064; // Hashed value for UNFINISHED, which is used to mark room exits as unfinished and to be completed later.

const uint32_t  Room::RESPAWN_INTERVAL =    300;        // The minimum respawn time, in seconds, for Mobiles.

// The descriptions for different types of room scars.
const std::vector<std::vector<std::string>> Room::ROOM_SCAR_DESCS = {
    // Room scar type 0: blood.
    { "There are a few drops of blood on the ground nearby.",
      "There is a large splash of blood on the ground nearby.",
      "There are a few splashes of blood here and there.",
      "Splashes of blood coat the floor and nearby surfaces." },

    // Room scar type 1: burns.
    { "There are a few small burn marks here and there.",
      "There are a few scorch marks from fire nearby.",
      "The ground nearby is scorched and charred.",
      "The ground and nearby surfaces are badly charred and scorched." },

    // Room scar type 2: debris.
    { "There are a few worthless pieces of metal and debris scattered about.",
      "A few pieces of metal and other mechanical debris are strewn around.",
      "Pieces of scorched metal and other mechanical debris are strewn around.",
      "Large chunks of twisted metal and other mechanical debris are strewn all around." },

    // Room scar type 3: dirt.
    { "Some of the dirt nearby has been moved into an uneven mound.",
      "Several mounds of dirt are visible, the dirt recently unsettled.",
      "Someone has been busy, the dirt nearby churned up and uneven.",
      "The dirt has been dug up and moved around repeatedly, as if someone has been digging many holes here." },

    // Room scar type 4: vomit.
    { "There are some bits of half-digested food on the ground.",
      "Someone appears to have violently vomited nearby.",
      "Several splashes of vomit mar the ground.",
      "The area reeks of vomit, which appears to be splattered everywhere nearby." },

    // Room scar type 5: campfires.
    { "Some ashes and remains of a campfire litter the ground.",
      "The fading remains of a campfire burn nearby, casting flickering lights.",
      "A crackling campfire burns nearby, warming the area.",
      "A bright, crackling campfire burns cheerfully nearby, warming the area." },

    // Room scar type 6: water.
    { "A few drops of water glisten on the ground.",
      "A little water has been splashed around nearby.",
      "Someone seems to have splashed a lot of water around nearby.",
      "A great deal of water has been splashed around nearby, getting everything wet." }
};

// The SQL table construction string for the saved rooms.
const std::string   Room::SQL_ROOMS =   "CREATE TABLE rooms ( sql_id INTEGER PRIMARY KEY UNIQUE NOT NULL, id INTEGER UNIQUE NOT NULL, last_spawned_mobs INTEGER, scars TEXT, "
    "spawn_mobs TEXT, tags TEXT, link_tags TEXT, inventory INTEGER UNIQUE )";


Room::Room(std::string new_id) : m_inventory(std::make_shared<Inventory>()), m_last_spawned_mobs(0), m_light(0), m_security(Security::ANARCHY)
{
    if (new_id.size()) m_id = StrX::hash(new_id);
    else m_id = 0;

    for (unsigned int e = 0; e < ROOM_LINKS_MAX; e++)
        m_links[e] = 0;
}

// This Room was previously inactive, and has now become active.
void Room::activate()
{
    // nothing happens here yet
}

// Adds a scar to this room.
void Room::add_scar(ScarType type, int intensity)
{
    if (tag(RoomTag::WaterShallow) || tag(RoomTag::WaterDeep)) return;
    int pos = -1;
    for (unsigned int i = 0; i < m_scar_type.size(); i++)
    {
        if (m_scar_type.at(i) == type)
        {
            pos = i;
            break;
        }
    }

    unsigned int total_intensity = intensity;
    if (pos > -1) total_intensity += m_scar_intensity.at(pos);
    if (total_intensity > 250) total_intensity = 250;

    if (pos > -1) m_scar_intensity.at(pos) = total_intensity;
    else
    {
        m_scar_type.push_back(type);
        m_scar_intensity.push_back(total_intensity);
    }
}

// Adds a Mobile or List to the mobile spawn list.
void Room::add_mob_spawn(const std::string &id) { m_spawn_mobs.push_back(id); }

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

// This Room was previously active, and has now become inactive.
void Room::deactivate()
{
    // nothing happens here yet
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

// As above, but takes an integer link instead of an enum.
bool Room::fake_link(uint8_t dir) const { return fake_link(static_cast<Direction>(dir)); }

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

// Loads the Room and anything it contains.
void Room::load(std::shared_ptr<SQLite::Database> save_db)
{
    uint32_t inventory_id = 0;
    SQLite::Statement query(*save_db, "SELECT * FROM rooms WHERE id = ?");
    query.bind(1, m_id);
    if (query.executeStep())
    {
        inventory_id = query.getColumn("inventory").getUInt();
        if (!query.isColumnNull("last_spawned_mobs")) m_last_spawned_mobs = query.getColumn("last_spawned_mobs").getUInt();
        if (!query.isColumnNull("link_tags"))
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
        if (!query.isColumnNull("scars"))
        {
            std::string scar_str = query.getColumn("scars").getString();
            std::vector<std::string> scar_pairs = StrX::string_explode(scar_str, ",");
            for (unsigned int i = 0; i < scar_pairs.size(); i++)
            {
                std::vector<std::string> pair_explode = StrX::string_explode(scar_pairs.at(i), ";");
                if (pair_explode.size() != 2) throw std::runtime_error("Malformed room scars data.");
                m_scar_type.push_back(static_cast<ScarType>(StrX::htoi(pair_explode.at(0))));
                m_scar_intensity.push_back(StrX::htoi(pair_explode.at(1)));
            }
        }
        if (!query.isColumnNull("tags")) StrX::string_to_tags(query.getColumn("tags").getString(), m_tags);

        // Make sure this goes *after* loading tags.
        if (tag(RoomTag::MobSpawnListChanged))
        {
            m_spawn_mobs.clear();
            if (!query.isColumnNull("spawn_mobs")) m_spawn_mobs = StrX::string_explode(query.getColumn("spawn_mobs").getString(), " ");
        }
    }
    if (inventory_id) m_inventory->load(save_db, inventory_id);
}

// Returns the Room's full or short name.
std::string Room::name(bool short_name) const { return (short_name ? m_name_short : m_name); }

// Respawn Mobiles in this Room, if possible.
void Room::respawn_mobs()
{
    if (!m_spawn_mobs.size()) return;       // Do nothing if there's nothing to spawn.
    if (m_id == core()->world()->player()->location()) return;  // Do nothing if the player is standing here.
    if (tag(RoomTag::MobSpawned)) return;   // Do nothing if a Mobile has already spawned here.
    if (m_last_spawned_mobs && core()->world()->time_weather()->time_passed_since(m_last_spawned_mobs) < RESPAWN_INTERVAL) return;    // Do nothing if the respawn timer isn't up.

    // Set the respawn timer!
    m_last_spawned_mobs = core()->world()->time_weather()->time_passed();

    // Pick a Mobile to spawn here.
    std::string spawn_str = m_spawn_mobs.at(core()->rng()->rnd(m_spawn_mobs.size()) - 1);
    if (spawn_str.size() && spawn_str[0] == '#') spawn_str = core()->world()->get_list(spawn_str.substr(1))->rnd().str; // If it's a list, pick an entry.
    if (!spawn_str.size() || spawn_str == "-") return;   // If for some reason we pick a blank entry, just do nothing. Yes, we updated the spawn timer, that's fine.

    // Spawn the Mobile!
    auto new_mob = core()->world()->get_mob(spawn_str);
    new_mob->set_location(m_id);
    new_mob->set_spawn_room(m_id);
    core()->world()->add_mobile(new_mob);
    set_tag(RoomTag::MobSpawned);
}

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

    if (!tags.size() && link_tags == ",,,,,,,,," && !m_scar_type.size()) return;

    SQLite::Statement room_query(*save_db, "INSERT INTO rooms (id, inventory, last_spawned_mobs, link_tags, scars, spawn_mobs, sql_id, tags) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    room_query.bind(1, m_id);
    if (inventory_id) room_query.bind(2, inventory_id);
    if (m_last_spawned_mobs) room_query.bind(3, m_last_spawned_mobs);
    if (link_tags != ",,,,,,,,,") room_query.bind(4, link_tags);
    if (m_scar_type.size())
    {
        std::string scar_str;
        for (unsigned int i = 0; i < m_scar_type.size(); i++)
        {
            scar_str += StrX::itoh(static_cast<int>(m_scar_type.at(i)), 1) + ";" + StrX::itoh(m_scar_intensity.at(i), 1);
            if (i < m_scar_type.size() - 1) scar_str += ",";
        }
        room_query.bind(5, scar_str);
    }
    if (tag(RoomTag::MobSpawnListChanged) && m_spawn_mobs.size()) room_query.bind(6, StrX::collapse_vector(m_spawn_mobs));
    room_query.bind(7, core()->sql_unique_id());
    if (tags.size()) room_query.bind(8, tags);
    room_query.exec();
}

// Returns the description of any room scars present.
std::string Room::scar_desc() const
{
    std::string scars;
    for (unsigned int i = 0; i < m_scar_type.size(); i++)
    {
        const int intensity = m_scar_intensity.at(i);
        uint32_t vec_pos = 0;
        if (intensity >= 20) vec_pos =  3;
        else if (intensity >= 10) vec_pos = 2;
        else if (intensity >= 5) vec_pos = 1;
        scars += ROOM_SCAR_DESCS.at(static_cast<uint32_t>(m_scar_type.at(i))).at(vec_pos) + " ";
    }
    if (tag(RoomTag::PermaCampfire) && !tag(RoomTag::HideCampfireScar)) scars += ROOM_SCAR_DESCS.at(static_cast<uint32_t>(ScarType::CAMPFIRE)).at(3) + " ";
    if (scars.size()) scars.pop_back();
    return scars;
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
