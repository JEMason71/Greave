// actions/status.hpp -- Meta status actions, such as checking the player's score or condition.
/// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Mobile;   // defined in world/mobile.hpp


class ActionStatus
{
public:
    static void score();    // Check the player's current total score.
    static void skills();   // Checks the player's skill levels.
    static void time();     // Determines the current time of day.
    static void weather();  // Checks the nearby weather.
};
