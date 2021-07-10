// actions/rest.cpp -- Commands that allow resting and sleeping for specified periods of time.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/rest.hpp"
#include "core/core.hpp"
#include "core/parser.hpp"
#include "core/strx.hpp"
#include "world/player.hpp"
#include "world/time-weather.hpp"
#include "world/world.hpp"


// Rests for a specified amount of time.
void ActionRest::rest(const std::string&, const std::vector<std::string> &words)
{
    int time_rest = TimeWeather::MINUTE * 10;
    if (words.size())
    {
        if (words.size() < 2 || words.size() > 2 || !StrX::is_number(words.at(0)))
        {
            core()->message("{y}Please specify {Y}how long to rest {y}(e.g. wait 3 hours).");
            return;
        }
        int num = core()->parser()->parse_int(words.at(0));
        const std::string time_str = StrX::str_tolower(words.at(1));
        const std::string time_substr = (time_str.size() >= 3 ? time_str.substr(0, 3) : "");
        if (time_substr == "hou") time_rest = TimeWeather::HOUR * num;
        else if (time_substr == "min") time_rest = TimeWeather::MINUTE * num;
        else if (time_substr == "sec") time_rest = TimeWeather::SECOND * num;
        else
        {
            core()->message("{y}Please specify {Y}how long to rest {y}(e.g. wait 3 hours).");
            return;
        }
    }
    if (time_rest > TimeWeather::HOUR * 24) time_rest = TimeWeather::HOUR * 24;
    core()->message("{u}Time passes....");
    core()->world()->player()->set_tag(MobileTag::Resting);
    const bool uninterrupted = core()->world()->player()->pass_time(time_rest);
    core()->world()->player()->clear_tag(MobileTag::Resting);
    if (!uninterrupted) core()->message("{c}You awaken with a start!");
}
