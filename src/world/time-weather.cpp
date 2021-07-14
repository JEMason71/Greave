// world/time-weather.cpp -- The time and weather system.
// Copyright (c) 2021 Raine "Gravecat" Simmons. All rights reserved.
// Weather system originally based on Keran's MUSH/MUX Weather and Time Code Package Version 4.0 beta, copyright (c) 1996-1998 Keran.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "3rdparty/yaml-cpp/yaml.h"
#include "actions/ai.hpp"
#include "core/core.hpp"
#include "core/random.hpp"
#include "core/strx.hpp"
#include "world/player.hpp"
#include "world/room.hpp"
#include "world/time-weather.hpp"
#include "world/world.hpp"


// SQL table construction string for the heartbeat timers.
const std::string TimeWeather::SQL_HEARTBEATS = "CREATE TABLE heartbeats ( id INTEGER PRIMARY KEY UNIQUE NOT NULL, count INTEGER NOT NULL )";

// SQL table construction string for time and weather data.
const std::string TimeWeather::SQL_TIME_WEATHER = "CREATE TABLE time_weather ( day INTEGER NOT NULL, moon INTEGER NOT NULL, subsecond REAL NOT NULL, time INTEGER PRIMARY KEY UNIQUE NOT NULL, time_total INTEGER NOT NULL, weather INTEGER NOT NULL )";

const int   TimeWeather::LUNAR_CYCLE_DAYS =     29;     // How many days are in a lunar cycle?
const float TimeWeather::UNINTERRUPTABLE_TIME = 5.0f;   // The maximum amount of time for an action that cannot be interrupted.
const int   TimeWeather::XP_WHILE_ENCUMBERED =  1;      // How much XP to grant per carry tick for encumbered players.

// The heartbeat timers, for triggering various events at periodic intervals.
const uint32_t TimeWeather::HEARTBEAT_TIMERS[TimeWeather::Heartbeat::_TOTAL] = {
    10 * Time::SECOND,  // BUFFS, for ticking down buffs/debuffs on Mobiles and the Player.
    17 * Time::MINUTE,  // CARRY, increases the player's hauling skill if they're heavily loaded.
    16 * Time::MINUTE,  // DISEASE, ticks diseases and reduces blood toxicity in the player's body.
    2 * Time::MINUTE,   // HP_REGEN, causes health to regenerate over time.
    432 * Time::MINUTE, // HUNGER. Pretty slow, as you can live for a long time without food.
    30 * Time::MINUTE,  // MOBILE_SPAWN, used to trigger Mobiles (re)spawning.
    2 * Time::SECOND,   // MP_REGEN, regenerates mana points over time.
    10 * Time::MINUTE,  // ROOM_SCARS, for decreasing the intensity of room scars.
    1 * Time::SECOND,   // SP_REGEN, regenerates stamina points over time.
    311 * Time::MINUTE, // THIRST. More rapid than hunger.
    5 * Time::MINUTE,   // WILDERNESS_SPAWN, for spawning beasts in the wilderness.
};


// Constructor, sets default values.
TimeWeather::TimeWeather() : m_day(80), m_moon(1), m_time(39660), m_time_passed(0), m_subsecond(0), m_weather(Weather::FAIR)
{
    m_weather_change_map.resize(9);
    try
    {
        const YAML::Node yaml_weather = YAML::LoadFile("data/misc/weather.yml");
        for (auto w : yaml_weather)
        {
            const std::string id = w.first.as<std::string>();
            const std::string text = w.second.as<std::string>();
            if (id.size() == 5 && id.substr(0, 4) == "WMAP")
            {
                const int map_id = id[4] - '0';
                if (map_id < 0 || map_id > 8) throw std::runtime_error("Invalid weather map strings.");
                m_weather_change_map.at(map_id) = StrX::decode_compressed_string(text);
            }
            else m_tw_string_map.insert(std::pair<std::string, std::string>(id, text));
        }
    }
    catch (std::exception& e)
    {
        throw std::runtime_error("Error while loading data/misc/weather.yml: " + std::string(e.what()));
    }

    // Reset all the heartbeats.
    for (unsigned int h = 0; h < TimeWeather::Heartbeat::_TOTAL; h++)
        m_heartbeats[h] = HEARTBEAT_TIMERS[h];
}

// Gets the current season.
TimeWeather::Season TimeWeather::current_season() const
{
    if (m_day > 364) throw std::runtime_error("Impossible day specified!");
    if (m_day < 79) return Season::WINTER;
    else if (m_day < 172) return Season::SPRING;
    else if (m_day <= 266) return Season::SUMMER;
    else if (m_day <= 355) return Season::AUTUMN;
    else return Season::WINTER;
}

// Returns the name of the current day of the week.
std::string TimeWeather::day_name() const
{
    int temp_day = m_day;
    while (temp_day > 7) temp_day -= 7;
    switch (temp_day)
    {
        case 1: return "Sunsday";       // Sunday
        case 2: return "Moonsday";      // Monday
        case 3: return "Heavensday";    // Tuesday
        case 4: return "Oathsday";      // Wednesday
        case 5: return "Crownsday";     // Thursday
        case 6: return "Swordsday";     // Friday
        case 7: return "Silversday";    // Saturday
    }
    return "";  // Just to make the compiler shut up.
}

// Returns the current day of the month.
int TimeWeather::day_of_month() const
{
    if (m_day <= 28) return m_day;
    int temp_day = m_day;
    while (temp_day > 28) temp_day -= 28;
    return temp_day;
}

// Returns the day of the month in the form of a string like "1st" or "19th".
std::string TimeWeather::day_of_month_string() const
{
    const int dom = day_of_month();
    std::string str = std::to_string(dom);
    if (dom == 1 || dom == 21) return str + "st";
    else if (dom == 2 || dom == 22) return str += "nd";
    else if (dom == 3 || dom == 23) return str += "rd";
    return str + "th";
}

// Fixes weather for a specified season, to account for unavailable weather types.
TimeWeather::Weather TimeWeather::fix_weather(TimeWeather::Weather weather, TimeWeather::Season season) const
{
    if (season == Season::SPRING && weather == Weather::SLEET) weather = Weather::RAIN;
    else if (season == Season::SUMMER || season == Season::AUTUMN)
    {
        if (weather == Weather::BLIZZARD) weather = Weather::STORMY;
        else if (weather == Weather::LIGHTSNOW || weather == Weather::SLEET) weather = Weather::RAIN;
    }
    return weather;
}

// Gets the current weather, runs fix_weather() internally.
TimeWeather::Weather TimeWeather::get_weather() const { return fix_weather(m_weather, current_season()); }

// Increases a specified heartbeat timer.
void TimeWeather::increase_heartbeat(Heartbeat beat, int count)
{
    if (beat >= Heartbeat::_TOTAL) throw std::runtime_error("Invalid heartbeat ID!");
    m_heartbeats[static_cast<int>(beat)] += count;
}

// Checks if a given heartbeat is ready to trigger, and resets its counter.
bool TimeWeather::heartbeat_ready(Heartbeat beat)
{
    if (beat >= Heartbeat::_TOTAL) throw std::runtime_error("Invalid heartbeat ID!");
    if (m_heartbeats[beat] >= HEARTBEAT_TIMERS[beat])
    {
        m_heartbeats[beat] = 0;
        return true;
    }
    return false;
}

// Checks whether it's light or dark right now.
TimeWeather::LightDark TimeWeather::light_dark() const
{
    if (m_time >= 1285 * Time::MINUTE) return LightDark::NIGHT;
    else if (m_time >= 1140 * Time::MINUTE) return LightDark::DARK;
    else if (m_time >= 420 * Time::MINUTE) return LightDark::LIGHT;
    else if (m_time >= 277 * Time::MINUTE) return LightDark::DARK;
    else return LightDark::NIGHT;
}

// Loads the time/weather data from disk.
void TimeWeather::load(std::shared_ptr<SQLite::Database> save_db)
{
    SQLite::Statement query(*save_db, "SELECT * FROM time_weather");
    if (query.executeStep())
    {
        m_day = query.getColumn("day").getInt();
        m_moon = query.getColumn("moon").getInt();
        m_subsecond = query.getColumn("subsecond").getDouble();
        m_time = query.getColumn("time").getInt();
        m_time_passed = query.getColumn("time_total").getUInt();
        m_weather = static_cast<Weather>(query.getColumn("weather").getInt());
    }
    else throw std::runtime_error("Could not load time and weather data!");

    SQLite::Statement heartbeat_query(*save_db, "SELECT * FROM heartbeats");
    while (heartbeat_query.executeStep())
    {
        uint32_t id = heartbeat_query.getColumn("id").getUInt();
        if (id >= Heartbeat::_TOTAL) throw std::runtime_error("Invalid heartbeat data!");
        m_heartbeats[id] = heartbeat_query.getColumn("count").getUInt();
    }
}

// Returns the name of the current month.
std::string TimeWeather::month_name() const
{
    if (m_day <= 28) return "Harrowing";            // January
    else if (m_day <= 56) return "Shadows";         // February
    else if (m_day <= 84) return "the Lord";        // March
    else if (m_day <= 112) return "the Lady";       // April
    else if (m_day <= 140) return "the Fall";       // May
    else if (m_day <= 168) return "Fortune";        // June
    else if (m_day <= 196) return "Fire";           // Sol
    else if (m_day <= 224) return "Gold";           // July
    else if (m_day <= 252) return "Seeking";        // August
    else if (m_day <= 280) return "the Serpent";    // September
    else if (m_day <= 308) return "Crimson";        // October
    else if (m_day <= 336) return "King's Night";   // November
    else return "Frost";                            // December
}

// Gets the current lunar phase.
TimeWeather::LunarPhase TimeWeather::moon_phase() const
{
    switch (m_moon)
    {
        case 0: return LunarPhase::NEW;
        case 1: case 2: case 3: case 4: case 5: case 6: return LunarPhase::WAXING_CRESCENT;
        case 7: case 8: case 9: return LunarPhase::FIRST_QUARTER;
        case 10: case 11: case 12: case 13: case 14: return LunarPhase::WAXING_GIBBOUS;
        case 15: return LunarPhase::FULL;
        case 16: case 17: case 18: case 19: case 20: return LunarPhase::WANING_GIBBOUS;
        case 21: case 22: case 23: return LunarPhase::THIRD_QUARTER;
        case 24: case 25: case 26: case 27: case 28: return LunarPhase::WANING_CRESCENT;
        default: throw std::runtime_error("Impossible moon phase!");
    }
}

// Causes time to pass.
bool TimeWeather::pass_time(float seconds, bool interruptable)
{
    if (seconds <= UNINTERRUPTABLE_TIME) interruptable = false;
    const auto world = core()->world();
    const auto player = world->player();
    const auto room = world->get_room(player->location());
    const bool indoors = room->tag(RoomTag::Indoors);
    const bool can_see_outside = room->tag(RoomTag::CanSeeOutside);
    const bool player_is_resting = player->tag(MobileTag::Resting);

    // Determine how many seconds to pass.
    m_subsecond += seconds;
    int seconds_to_add = 0;
    if (m_subsecond >= 1.0f)
    {
        seconds_to_add = std::floor(m_subsecond);
        m_subsecond -= seconds_to_add;
    }

    int old_hp = player->hp();
    int old_hunger = player->hunger();
    int old_thirst = player->thirst();
    while (seconds_to_add--)
    {
        if (player->is_dead()) return false;    // Don't pass time if the player is dead.

        // Interrupt the action if the player takes damage.
        if (interruptable)
        {
            const int hp = player->hp();
            const int hunger = player->hunger();
            const int thirst = player->thirst();
            if (hp < old_hp || (hunger < old_hunger && hunger <= 6) || (thirst < old_thirst && thirst <= 6)) return false;
            old_hp = hp;
            old_hunger = hunger;
            old_thirst = thirst;
        }

        m_time_passed++;    // The total time passed in the game. This will loop every 136 years, but that's not a problem; see time_passed().

        // Increase all heartbeat timers.
        for (unsigned int h = 0; h < Heartbeat::_TOTAL; h++)
            m_heartbeats[h]++;

        // Update the time of day and weather.
        const bool show_weather_messages = (!indoors || can_see_outside);
        TimeOfDay old_time_of_day = time_of_day(true);
        int old_time = m_time;
        bool change_happened = false;
        std::string weather_msg;
        if (++m_time >= Time::DAY) m_time -= Time::DAY;
        if (m_time >= 420 * Time::MINUTE && old_time < 420 * Time::MINUTE)   // Trigger moon-phase changing and day-of-year changing at dawn, not midnight.
        {
            if (++m_day > 364) m_day = 1;
            if (++m_moon >= LUNAR_CYCLE_DAYS) m_moon = 0;
            core()->message("{B}It is now " + day_name() + ", the " + day_of_month_string() + " day of " +  month_name() + ".");
        }
        old_time = m_time;
        if (time_of_day(true) != old_time_of_day)
        {
            weather_msg = "";
            old_time_of_day = time_of_day(true);
            trigger_event(current_season(), &weather_msg, !show_weather_messages);
            change_happened = show_weather_messages;
        }
        if (change_happened && !player_is_resting) core()->message(weather_message_colour() + weather_msg.substr(1));

        // Runs the AI on all active mobiles.
        AI::tick_mobs();
        if (player->is_dead()) return true;

        std::set<uint32_t> active_rooms;    // This starts empty, but can be re-used if multiple heartbeats need to check active rooms.

        // Scan through all active rooms, respawning NPCs if needed.
        if (heartbeat_ready(Heartbeat::MOBILE_SPAWN))
        {
            if (!active_rooms.size()) active_rooms = world->active_rooms();
            for (auto room_id : active_rooms)
            {
                const auto room = world->get_room(room_id);
                room->respawn_mobs();
            }
        }

        // Spawn mobiles in wilderness rooms.
        if (heartbeat_ready(Heartbeat::WILDERNESS_SPAWN)) world->wilderness_spawns();

        // Reduce room scars on active rooms.
        if (heartbeat_ready(Heartbeat::ROOM_SCARS))
        {
            if (!active_rooms.size()) active_rooms = world->active_rooms();
            for (auto room_id : active_rooms)
            {
                const auto room = world->get_room(room_id);
                room->decay_scars();
            }
        }

        // Reduce timers on buffs for all Mobiles and the Player.
        if (heartbeat_ready(Heartbeat::BUFFS))
        {
            player->tick_buffs();
            if (player->is_dead()) return true;
            for (size_t m = 0; m < world->mob_count(); m++)
                world->mob_vec(m)->tick_buffs();
        }

        // Increases the player's hunger.
        if (heartbeat_ready(Heartbeat::HUNGER))
        {
            player->hunger_tick();
            if (player->is_dead()) return true;
        }

        // Increases the player's thirst.
        if (heartbeat_ready(Heartbeat::THIRST))
        {
            player->thirst_tick();
            if (player->is_dead()) return true;
        }

        // Regenerates hit points over time.
        if (heartbeat_ready(Heartbeat::HP_REGEN))
        {
            player->tick_hp_regen();
            for (unsigned int i = 0; i < world->mob_count(); i++)
                world->mob_vec(i)->tick_hp_regen();
        }

        // Regenerates stamina points over time.
        if (heartbeat_ready(Heartbeat::SP_REGEN)) player->tick_sp_regen();

        // Regenerates mana points over time.
        if (heartbeat_ready(Heartbeat::MP_REGEN)) player->tick_mp_regen();

        // Ticks diseases and reduces blood toxicity.
        if (heartbeat_ready(Heartbeat::DISEASE))
        {
            player->tick_blood_tox();
        }

        // Increases the player's hauling skill if they are over-encumbered.
        if (heartbeat_ready(Heartbeat::CARRY))
            if (player->carry_weight() > std::round(static_cast<float>(player->max_carry()) * 0.75f)) player->gain_skill_xp("HAULING", XP_WHILE_ENCUMBERED);
    }

    return true;
}

// Saves the time/weather data to disk.
void TimeWeather::save(std::shared_ptr<SQLite::Database> save_db) const
{
    SQLite::Statement query(*save_db, "INSERT INTO time_weather ( day, moon, subsecond, time, time_total, weather ) VALUES ( :day, :moon, :subsecond, :time, :time_total, :weather )");
    query.bind(":day", m_day);
    query.bind(":moon", m_moon);
    query.bind(":subsecond", m_subsecond);
    query.bind(":time", m_time);
    query.bind(":time_total", m_time_passed);
    query.bind(":weather", static_cast<int>(m_weather));
    query.exec();

    for (unsigned int h = 0; h < Heartbeat::_TOTAL; h++)
    {
        SQLite::Statement heartbeat_query(*save_db, "INSERT INTO heartbeats ( id, count ) VALUES ( :id, :count )");
        heartbeat_query.bind(":id", h);
        heartbeat_query.bind(":count", m_heartbeats[h]);
        heartbeat_query.exec();
    }
}

// Converts a season enum to a string.
std::string TimeWeather::season_str(TimeWeather::Season season) const
{
    switch (season)
    {
        case Season::WINTER: return "WINTER";
        case Season::SPRING: return "SPRING";
        case Season::AUTUMN: return "AUTUMN";
        case Season::SUMMER: return "SUMMER";
        default: throw std::runtime_error("Invalid season specified!");
    }
}

// Returns the current time of day (morning, day, dusk, night).
TimeWeather::TimeOfDay TimeWeather::time_of_day(bool fine) const
{
    if (fine)
    {
        if (m_time >= 1380 * Time::MINUTE) return TimeOfDay::MIDNIGHT;
        else if (m_time >= 1260 * Time::MINUTE) return TimeOfDay::NIGHT;
        else if (m_time >= 1140 * Time::MINUTE) return TimeOfDay::DUSK;
        else if (m_time >= 1020 * Time::MINUTE) return TimeOfDay::SUNSET;
        else if (m_time >= 660 * Time::MINUTE) return TimeOfDay::NOON;
        else if (m_time >= 540 * Time::MINUTE) return TimeOfDay::MORNING;
        else if (m_time >= 420 * Time::MINUTE) return TimeOfDay::SUNRISE;
        else if (m_time >= 300 * Time::MINUTE) return TimeOfDay::DAWN;
        return TimeOfDay::MIDNIGHT;
    } else
    {
        if (m_time >= 1380 * Time::MINUTE) return TimeOfDay::NIGHT;
        if (m_time >= 1140 * Time::MINUTE) return TimeOfDay::DUSK;
        if (m_time >= 540 * Time::MINUTE) return TimeOfDay::DAY;
        if (m_time >= 300 * Time::MINUTE) return TimeOfDay::DAWN;
        return TimeOfDay::NIGHT;
    }
}

// Returns the exact time of day.
int TimeWeather::time_of_day_exact() const { return m_time; }

// Returns the current time of day as a string.
std::string TimeWeather::time_of_day_str(bool fine) const
{
    if (fine)
    {
        if (m_time >= 1380 * Time::MINUTE) return "MIDNIGHT";
        else if (m_time >= 1260 * Time::MINUTE) return "NIGHT";
        else if (m_time >= 1140 * Time::MINUTE) return "DUSK";
        else if (m_time >= 1020 * Time::MINUTE) return "SUNSET";
        else if (m_time >= 660 * Time::MINUTE) return "NOON";
        else if (m_time >= 540 * Time::MINUTE) return "MORNING";
        else if (m_time >= 420 * Time::MINUTE) return "SUNRISE";
        else if (m_time >= 300 * Time::MINUTE) return "DAWN";
        return "NIGHT";
    } else
    {
        if (m_time >= 1380 * Time::MINUTE) return "NIGHT";
        if (m_time >= 1140 * Time::MINUTE) return "DUSK";
        if (m_time >= 540 * Time::MINUTE) return "DAY";
        if (m_time >= 300 * Time::MINUTE) return "DAWN";
        return "NIGHT";
    }
}

void TimeWeather::trigger_event(TimeWeather::Season season, std::string *message_to_append, bool silent)
{
    const std::string weather_map = m_weather_change_map.at(static_cast<int>(m_weather));
    const char new_weather = weather_map[core()->rng()->rnd(0, weather_map.size() - 1)];
    switch (new_weather)
    {
        case 'c': m_weather = Weather::CLEAR; break;
        case 'f': m_weather = Weather::FAIR; break;
        case 'r': m_weather = Weather::RAIN; break;
        case 'F': m_weather = Weather::FOG; break;
        case 'S': m_weather = Weather::STORMY; break;
        case 'o': m_weather = Weather::OVERCAST; break;
        case 'b': m_weather = Weather::BLIZZARD; break;
        case 'l': m_weather = Weather::LIGHTSNOW; break;
        case 'L': m_weather = Weather::SLEET; break;
    }
    if (silent) return;

    // Display an appropriate message for the changing time/weather, if we're outdoors.
    const std::shared_ptr<Room> room = core()->world()->get_room(core()->world()->player()->location());
    const bool indoors = room->tag(RoomTag::Indoors);
    const bool can_see_outside = room->tag(RoomTag::CanSeeOutside);
    if (indoors && !can_see_outside) return;
    const std::string time_message = m_tw_string_map.at(time_of_day_str(true) + "_" + weather_str(fix_weather(m_weather, season)) + (indoors ? "_INDOORS" : ""));
    if (message_to_append) *message_to_append += " " + time_message;
    else core()->message(weather_message_colour() + time_message);
}

// Returns the total amount of seconds that passed in the game.
uint32_t TimeWeather::time_passed() const { return m_time_passed; }

// Checks how much time has passed since a given time integer. Handles integer overflow loops.
uint32_t TimeWeather::time_passed_since(uint32_t since) const
{
    // If the total time hasn't looped yet, no problem! This is easy!
    if (since <= m_time_passed) return m_time_passed - since;

    // If not, we'll try to work around the overflow. This ain't great if we overflowed twice, but shouldn't cause too many problems.
    return UINT32_MAX - since + m_time_passed;
}

// Returns a weather description for the current time/weather, based on the current season.
std::string TimeWeather::weather_desc() const { return weather_desc(current_season()); }

// Returns a weather description for the current time/weather, based on the specified season.
std::string TimeWeather::weather_desc(TimeWeather::Season season) const
{
    const std::shared_ptr<Room> room = core()->world()->get_room(core()->world()->player()->location());
    const bool trees = room->tag(RoomTag::Trees);
    const bool indoors = room->tag(RoomTag::Indoors);
    const Weather weather = fix_weather(m_weather, season);
    std::string desc = m_tw_string_map.at(season_str(season) + "_" + time_of_day_str(false) + "_" + weather_str(weather)  + (indoors ? "_INDOORS" : ""));
    if (trees)
    {
        std::string tree_time = "DAY";
        if (time_of_day(false) == TimeOfDay::DUSK || time_of_day(false) == TimeOfDay::NIGHT) tree_time = "NIGHT";
        desc += " " + m_tw_string_map.at(season_str(season) + "_" + tree_time + "_" + weather_str(weather) + "_TREES");
    }
    return desc;
}

// Returns a colour to be used for time/weather messages, based on the time of day.
std::string TimeWeather::weather_message_colour() const
{
    switch (light_dark())
    {
        case LightDark::DARK: return "{U}";
        case LightDark::LIGHT: return "{C}";
        case LightDark::NIGHT: return "{u}";
        default: return ""; // Just to keep the compiler happy.
    }
}

// Converts a weather integer to a string.
std::string TimeWeather::weather_str(TimeWeather::Weather weather) const
{
    switch (weather)
    {
        case Weather::BLIZZARD: return "BLIZZARD";
        case Weather::STORMY: return "STORMY";
        case Weather::RAIN: return "RAIN";
        case Weather::CLEAR: return "CLEAR";
        case Weather::FAIR: return "FAIR";
        case Weather::OVERCAST: return "OVERCAST";
        case Weather::FOG: return "FOG";
        case Weather::LIGHTSNOW: return "LIGHTSNOW";
        case Weather::SLEET: return "SLEET";
        default: throw std::runtime_error("Invalid weather specified: " + std::to_string(static_cast<int>(weather)));
    }
}
