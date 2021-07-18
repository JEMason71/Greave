// actions/travel.h -- Actions allowing the player and NPCs to move around the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_ACTIONS_TRAVEL_H_
#define GREAVE_ACTIONS_TRAVEL_H_

#include "world/mobile.h"
#include "world/room.h"

#include <memory>


class ActionTravel
{
public:
    static constexpr float  TRAVEL_TIME_DOUBLE =            120;    // The time (in seconds) it takes to travel across a double-length room link.
    static constexpr float  TRAVEL_TIME_NORMAL =            30;     // The time (in seconds) it takes to travel across a normal room link.
    static constexpr float  TRAVEL_TIME_TRIPLE =            480;    // The time (in seconds) it takes to travel across a triple-length room link.

    static bool travel(std::shared_ptr<Mobile> mob, Direction dir, bool confirm);   // Attempts to move from one Room to another.

private:
    static constexpr int    FALL_1_STOREY_BLEED =           5;      // Intensity for the bleed room scar from a one-storey fall.
    static constexpr int    FALL_1_STOREY_MIN_PERC =        20;     // Minimum % damage taken from falling one storey.
    static constexpr int    FALL_1_STOREY_RNG_PERC =        50;     // Extra RNG % damage from one-storey fall.
    static constexpr int    FALL_2_STOREY_BLEED =           10;
    static constexpr int    FALL_2_STOREY_MIN_PERC =        50;
    static constexpr int    FALL_2_STOREY_RNG_PERC =        70;
    static constexpr int    FALL_3_STOREY_BLEED =           20;
    static constexpr int    FALL_3_STOREY_MIN_PERC =        70;
    static constexpr int    FALL_3_STOREY_RNG_PERC =        100;
    static constexpr int    FALL_4_STOREY_BLEED =           25;
    static constexpr int    FALL_4_STOREY_MIN_PERC =        90;
    static constexpr int    FALL_4_STOREY_RNG_PERC =        200;
    static constexpr int    FALL_5_STOREY_BLEED =           30;
    static constexpr int    FALL_5_STOREY_MIN_PERC =        100;
    static constexpr int    FALL_5_STOREY_RNG_PERC =        500;
    static constexpr int    FALL_BLEED_DIVISOR_MAX =        20;     // The maximum amount of HP damage division from falling applied to each bleed tick.
    static constexpr int    FALL_BLEED_DIVISOR_MIN =        10;     // The minimum amount of HP damage division from falling applied to each bleed tick.
    static constexpr int    FALL_BLEED_INTENSITY_RANGE =    3;      // The variance range of the length of bleeds from falling.
    static constexpr float  XP_PER_SAFE_FALL_FAIL =         3;      // As below, but for failed attempts.
    static constexpr float  XP_PER_SAFE_FALL_SUCCESS =      8;      // How much base XP is gained from a successful safe-fall (multiplied by distance fallen).
};

#endif  // GREAVE_ACTIONS_TRAVEL_H_
