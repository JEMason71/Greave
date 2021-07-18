// actions/ai.h -- NPC AI actions and behaviour.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_ACTIONS_AI_H_
#define GREAVE_ACTIONS_AI_H_

#include <memory>

#include "world/mobile.h"


class AI
{
public:
    static void tick_mobs();    // Ticks all the mobiles in active rooms.

private:
    static constexpr int    AGGRO_CHANCE =                  60;     // 1 in X chance of starting a fight.
    static constexpr int    FLEE_DEBUFF_TIME =              48;     // The length of time the fleeing debuff lasts.
    static constexpr float  FLEE_TIME =                     60;     // The action time it takes to flee in terror.
    static constexpr float  STANCE_AGGRESSIVE_HP_PERCENT =  20;     // When a Mobile's target drops below this many hit points, they'll got to an aggressive stance.
    static constexpr float  STANCE_AGGRESSIVE_HP_RATIO =    1.3f;   // When a mobile's ratio of hit points lost compared to their target's hit points lost goes above this level, they'll go to an aggressive stance.
    static constexpr int    STANCE_COUNTER_CHANCE =         200;    // 1 in X chance to attempt to counter the target's choice of combat stance.
    static constexpr float  STANCE_DEFENSIVE_HP_PERCENT =   20;     // Mobiles will switch to defensive stance when their hit points drop below this percentage of maximum.
    static constexpr float  STANCE_DEFENSIVE_HP_RATIO =     0.7f;   // When a mobile's ratio of hit points lost compared to their target's hit points lost drops below this level, they'll go to a defensive stance.
    static constexpr int    STANCE_RANDOM_CHANCE =          500;    // 1 in X chance to pick a random stance, rather than making a strategic decision.
    static constexpr int    TRAVEL_CHANCE =                 300;    // 1 in X chance of traveling to another room.

    static void tick_mob(std::shared_ptr<Mobile> mob, uint32_t vec_pos);    // Processes AI for a specific active Mobile.
    static bool travel_randomly(std::shared_ptr<Mobile> mob, bool allow_dangerous_exits);   // Sends the Mobile in a random direction.
};

#endif  // GREAVE_ACTIONS_AI_H_
