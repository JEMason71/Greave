// actions/travel.hpp -- Actions allowing the player and NPCs to move around the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Mobile;   // defined in world/mobile.hpp


class ActionTravel
{
public:
    static bool travel(std::shared_ptr<Mobile> mob, Direction dir); // Attempts to move from one Room to another.

private:
    static const float  TRAVEL_TIME_DOUBLE; // The time (in seconds) it takes to travel across a double-length room link.
    static const float  TRAVEL_TIME_NORMAL; // The time (in seconds) it takes to travel across a normal room link.
    static const float  TRAVEL_TIME_TRIPLE; // The time (in seconds) it takes to travel across a triple-length room link.
};
