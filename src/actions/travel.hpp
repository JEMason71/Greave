// actions/travel.hpp -- Actions allowing the player and NPCs to move around the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Mobile;                   // defined in world/mobile.hpp
enum class Direction : uint8_t; // defined in world/room.hpp


class ActionTravel
{
public:
    static const float  TRAVEL_TIME_DOUBLE;         // The time (in seconds) it takes to travel across a double-length room link.
    static const float  TRAVEL_TIME_NORMAL;         // The time (in seconds) it takes to travel across a normal room link.
    static const float  TRAVEL_TIME_TRIPLE;         // The time (in seconds) it takes to travel across a triple-length room link.

    static bool travel(std::shared_ptr<Mobile> mob, Direction dir, bool confirm);   // Attempts to move from one Room to another.

private:
    static const int    FALL_1_STOREY_BLEED;        // Intensity for the bleed room scar from a one-storey fall.
    static const int    FALL_1_STOREY_MIN_PERC;     // Minimum % damage taken from falling one storey.
    static const int    FALL_1_STOREY_RNG_PERC;     // Extra RNG % damage from one-storey fall.
    static const int    FALL_2_STOREY_BLEED;
    static const int    FALL_2_STOREY_MIN_PERC;
    static const int    FALL_2_STOREY_RNG_PERC;
    static const int    FALL_3_STOREY_BLEED;
    static const int    FALL_3_STOREY_MIN_PERC;
    static const int    FALL_3_STOREY_RNG_PERC;
    static const int    FALL_4_STOREY_BLEED;
    static const int    FALL_4_STOREY_MIN_PERC;
    static const int    FALL_4_STOREY_RNG_PERC;
    static const int    FALL_5_STOREY_BLEED;
    static const int    FALL_5_STOREY_MIN_PERC;
    static const int    FALL_5_STOREY_RNG_PERC;
    static const int    FALL_BLEED_DIVISOR_MAX;     // The maximum amount of HP damage division from falling applied to each bleed tick.
    static const int    FALL_BLEED_DIVISOR_MIN;     // The minimum amount of HP damage division from falling applied to each bleed tick.
    static const int    FALL_BLEED_INTENSITY_RANGE; // The variance range of the length of bleeds from falling.
};
