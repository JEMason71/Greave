// actions/look.hpp -- Look around you. Just look around you.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Mobile;   // defined in world/mobile.hpp


class ActionLook
{
public:
    static void look(std::shared_ptr<Mobile> mob);      // Take a look around at your surroundings.
    static void time(std::shared_ptr<Mobile> mob);      // Determines the current time of day.
    static void weather(std::shared_ptr<Mobile> mob);   // Checks the nearby weather.
};
