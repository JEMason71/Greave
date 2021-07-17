// actions/rest.h -- Commands that allow resting and sleeping for specified periods of time.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once

#include <string>
#include <vector>


class ActionRest
{
public:
    static void rest(const std::string &word, const std::vector<std::string> &words, bool confirm); // Rests for a specified amount of time.
};
