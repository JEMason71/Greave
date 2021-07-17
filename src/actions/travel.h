// actions/travel.h -- Actions allowing the player and NPCs to move around the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_ACTIONS_TRAVEL_H_
#define GREAVE_ACTIONS_TRAVEL_H_

#include <memory>

#include "world/mobile.h"
#include "world/room.h"


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
    static const float  XP_PER_SAFE_FALL_FAIL;      // As below, but for failed attempts.
    static const float  XP_PER_SAFE_FALL_SUCCESS;   // How much base XP is gained from a successful safe-fall (multiplied by distance fallen).
};

#endif  // GREAVE_ACTIONS_TRAVEL_H_
