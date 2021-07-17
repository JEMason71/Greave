// actions/ai.h -- NPC AI actions and behaviour.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once

#include <memory>

#include "world/mobile.h"


class AI
{
public:
    static void tick_mobs();    // Ticks all the mobiles in active rooms.

private:
    static const int    AGGRO_CHANCE;                   // 1 in X chance of starting a fight.
    static const int    FLEE_DEBUFF_TIME;               // The length of time the fleeing debuff lasts.
    static const float  FLEE_TIME;                      // The action time it takes to flee in terror.
    static const int    TRAVEL_CHANCE;                  // 1 in X chance of traveling to another room.
    static const int    STANCE_AGGRESSIVE_HP_PERCENT;   // When a Mobile's target drops below this many hit points, they'll got to an aggressive stance.
                        // When a mobile's ratio of hit points lost compared to their target's hit points lost goes above this level, they'll go to an aggressive stance.
    static const float  STANCE_AGGRESSIVE_HP_RATIO;
    static const int    STANCE_COUNTER_CHANCE;          // 1 in X chance to attempt to counter the target's choice of combat stance.
    static const int    STANCE_DEFENSIVE_HP_PERCENT;    // Mobiles will switch to defensive stance when their hit points drop below this percentage of maximum.
                        // When a mobile's ratio of hit points lost compared to their target's hit points lost drops below this level, they'll go to a defensive stance.
    static const float  STANCE_DEFENSIVE_HP_RATIO;
    static const int    STANCE_RANDOM_CHANCE;           // 1 in X chance to pick a random stance, rather than making a strategic decision.

    static void tick_mob(std::shared_ptr<Mobile> mob, uint32_t vec_pos);    // Processes AI for a specific active Mobile.
    static bool travel_randomly(std::shared_ptr<Mobile> mob, bool allow_dangerous_exits);   // Sends the Mobile in a random direction.
};
