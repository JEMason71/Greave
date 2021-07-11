// actions/status.cpp -- Meta status actions, such as checking the player's score or condition.
/// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/status.hpp"
#include "core/core.hpp"
#include "core/strx.hpp"
#include "world/player.hpp"
#include "world/room.hpp"
#include "world/time-weather.hpp"
#include "world/world.hpp"


// Check the player's current total score.
void ActionStatus::score()
{
    core()->message("{U}Your current score is {C}" + StrX::intostr_pretty(core()->world()->player()->score()) + "{U}.");
}

// Checks the player's skill levels.
void ActionStatus::skills()
{
    const auto player = core()->world()->player();
    const auto skill_map = player->skill_map();
    std::vector<std::string> skill_names;
    std::vector<int> skill_levels;

    for (auto const &skill : skill_map)
    {
        if (skill.second)
        {
            skill_names.push_back(core()->world()->get_skill_name(skill.first));
            skill_levels.push_back(skill.second);
        }
    }
    if (!skill_levels.size())
    {
        core()->message("{y}You have no particular skills.");
        return;
    }

    // Sort the list of skills with the highest-level skill first. There's probably a better way to do this (feel free to improve this code if the urge takes you),
    // but for now I'm going to just go with a lazy and simple method and bubble-sort it by hand.
    if (skill_levels.size() > 1)
    {
        bool rearranged;
        do
        {
            rearranged = false;
            for (unsigned int i = 0; i < skill_levels.size() - 1; i++)
            {
                if (skill_levels.at(i + 1) > skill_levels.at(i))
                {
                    const int temp_level = skill_levels.at(i);
                    const std::string temp_name = skill_names.at(i);
                    skill_levels.at(i) = skill_levels.at(i + 1);
                    skill_levels.at(i + 1) = temp_level;
                    skill_names.at(i) = skill_names.at(i + 1);
                    skill_names.at(i + 1) = temp_name;
                    rearranged = true;
                }
            }
        }
        while (rearranged);
    }

    std::string skill_str;
    for (unsigned int i = 0; i < skill_levels.size(); i++)
    {
        if (i == skill_levels.size() - 1 && skill_levels.size() > 1) skill_str += " and ";
        else if (i > 0) skill_str += ", ";
        skill_str += "{C}" + skill_names.at(i) + " {U}(" + std::to_string(skill_levels.at(i)) + ")";
    }
    core()->message("{U}Your skills include " + skill_str + ".");
}

// Checks the player's overall status.
void ActionStatus::status()
{
    const auto player = core()->world()->player();
    const auto room = core()->world()->get_room(player->location());
    const bool indoors = room->tag(RoomTag::Indoors) || room->tag(RoomTag::Underground);
    const bool can_see_outside = !indoors || room->tag(RoomTag::CanSeeOutside);
    const std::string time_str = time(false);
    const std::string temp_str = temperature(false);
    std::string weather_str;
    switch (core()->world()->time_weather()->get_weather())
    {
        case TimeWeather::Weather::BLIZZARD: weather_str = "{C}A {U}raging blizzard {C}blows around " + std::string(indoors ? "you!" : "outside!"); break;
        case TimeWeather::Weather::STORMY: weather_str = "{C}Thunder rumbles in the sky as a {U}furious storm {C}rages" + std::string(indoors ? " outside!" : "!"); break;
        case TimeWeather::Weather::RAIN: weather_str = "{C}{U}Rain{C} lashes down from the sky" + std::string(indoors ? " outside!" : "!"); break;
        case TimeWeather::Weather::CLEAR: weather_str = "{C}The sky " + std::string(indoors ? "outside " : "") + "is {G}clear{C}."; break;
        case TimeWeather::Weather::FAIR: weather_str = "{C}The weather " + std::string(indoors ? "outside " : "") + "is {G}fair{C}."; break;
        case TimeWeather::Weather::OVERCAST: weather_str = "{C}The sky " + std::string(indoors ? "outside " : "") + "is {w}cloudy and overcast{C}."; break;
        case TimeWeather::Weather::FOG: weather_str = "{C}The world " + std::string(indoors ? "outside " : "") + "is wreathed in {w}thick fog{C}."; break;
        case TimeWeather::Weather::LIGHTSNOW: weather_str = "{C}{W}White snow {C}falls gently from the sky" + std::string(indoors ? "outside." : "."); break;
        case TimeWeather::Weather::SLEET: weather_str = "{c}Icy-cold sleet {C}falls angrily from the sky" + std::string(indoors ? "outside." : "."); break;
    }
    core()->message(time_str + " " + (can_see_outside ? weather_str + " " : "") + temp_str);

    std::string hunger_str, thirst_str, status_line;
    switch (player->hunger())
    {
        case 1: case 2: hunger_str = "{Y}You are starving to death!"; break;
        case 3: case 4: hunger_str = "{Y}You almost collapse from the hunger pain!"; break;
        case 5: case 6: hunger_str = "{Y}You are desperately hungry!"; break;
        case 7: case 8: hunger_str = "{Y}You are ravenously hungry!"; break;
        case 9: case 10: hunger_str = "{y}Your stomach rumbles loudly!"; break;
        case 11: case 12: hunger_str = "{y}Your stomach rumbles quietly."; break;
        case 13: case 14: hunger_str = "{y}You're starting to feel peckish."; break;
        default: hunger_str = "{y}You don't feel hungry right now.";
    }
    switch (player->thirst())
    {
        case 1: case 2: thirst_str = "{U}You are dying of dehydration!"; break;
        case 3: case 4: thirst_str = "{U}Your throat is so parched it's painful!"; break;
        case 5: case 6: thirst_str = "{U}You are desperately thirsty!"; break;
        case 7: case 8: thirst_str = "{U}You are extremely thirsty!"; break;
        case 9: case 10: thirst_str = "{u}Your motuth feels very dry."; break;
        case 11: case 12: thirst_str = "{u}You really want something to drink."; break;
        case 13: case 14: thirst_str = "{u}You're starting to feel a little thirsty."; break;
        default: thirst_str = "{u}You don't feel thirsty right now.";
    }
    if (player->hunger() > 14 && player->thirst() > 14) status_line = "{g}You don't feel hungry or thirsty right now.";
    else status_line = hunger_str + " " + thirst_str;

    if (player->has_buff(Buff::Type::BLEED)) status_line += " {R}You are bleeding quite badly!";
    if (player->has_buff(Buff::Type::POISON)) status_line += " {G}Deadly poison runs through your veins!";
    if (player->has_buff(Buff::Type::RECENT_DAMAGE)) status_line += " {r}You have recently taken damage, inhibiting your natural healing.";
    core()->message(status_line);
}

// Determines the current time of day.
std::string ActionStatus::time(bool print)
{
    const auto player = core()->world()->player();
    const auto time_weather = core()->world()->time_weather();
    const auto room = core()->world()->get_room(player->location());
    const bool indoors = room->tag(RoomTag::Indoors);
    const bool can_see_outside = room->tag(RoomTag::CanSeeOutside);
    const std::string date = time_weather->day_name() + ", the " + time_weather->day_of_month_string() + " day of " +  time_weather->month_name();

    std::string time_str;
    if (can_see_outside || !indoors) time_str = "It is now " + StrX::str_tolower(time_weather->time_of_day_str(true));
    else
    {
        std::string tod_str = time_weather->time_of_day_str(false);
        if (tod_str == "DAY") tod_str = "daytime";
        time_str = "It is around " + StrX::str_tolower(tod_str);
    }
    time_str = time_weather->weather_message_colour() + time_str + " on " + date + ".";
    if (print) core()->message(time_str);
    return time_str;
}

// Displays the current temperature.
std::string ActionStatus::temperature(bool print)
{
    const auto room = core()->world()->get_room(core()->world()->player()->location());
    const int player_temp = room->temperature(Room::TEMPERATURE_FLAG_WITH_PLAYER_BUFFS);
    const int room_temp = room->temperature(Room::TEMPERATURE_FLAG_IGNORE_PLAYER_CLOTHES);
    std::string temp_str = "{U}The temperature is ";
    switch (room_temp)
    {
        case 0: case 1: temp_str += "{C}freezing"; break;
        case 2: temp_str += "{C}cold"; break;
        case 3: temp_str += "{C}chilly"; break;
        case 4: case 5: temp_str += "{G}pleasant"; break;
        case 6: temp_str += "{Y}warm"; break;
        case 7: temp_str += "{Y}toasty"; break;
        case 8: temp_str += "{R}hot"; break;
        case 9: temp_str += "{R}searing"; break;
    }
    temp_str += "{U}, and you feel ";
    switch (player_temp)
    {
        case 0: case 1: temp_str += "{C}frozen"; break;
        case 2: temp_str += "{C}cold"; break;
        case 3: temp_str += "{C}chilly"; break;
        case 4: case 5: temp_str += "{G}fine"; break;
        case 6: temp_str += "{Y}warm"; break;
        case 7: temp_str += "{Y}toasty"; break;
        case 8: temp_str += "{R}hot"; break;
        case 9: temp_str += "{R}sweltering"; break;
    }
    temp_str += "{U}.";
    if (print) core()->message(temp_str);
    return temp_str;
}

// Checks the nearby weather.
void ActionStatus::weather()
{
    const auto player = core()->world()->player();
    const auto room = core()->world()->get_room(player->location());
    const bool indoors = room->tag(RoomTag::Indoors);
    const bool can_see_outside = room->tag(RoomTag::CanSeeOutside);
    if (indoors && !can_see_outside) core()->message("{y}You {Y}can't see {y}the weather outside from here.");
    else core()->message(core()->world()->time_weather()->weather_message_colour() + core()->world()->time_weather()->weather_desc());
    temperature();
}
