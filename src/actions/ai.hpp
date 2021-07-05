// actions/ai.hpp -- NPC AI actions and behaviour.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Mobile;   // defined in world/mobile.hpp


class AI
{
public:
    static void tick_mobs();    // Ticks all the mobiles in active rooms.

private:
    static const uint32_t   AGGRO_CHANCE;       // 1 in X chance of starting a fight.
    static const uint16_t   FLEE_DEBUFF_TIME;   // The length of time the fleeing debuff lasts.
    static const float      FLEE_TIME;          // The action time it takes to flee in terror.
    static const uint32_t   TRAVEL_CHANCE;      // 1 in X chance of traveling to another room.

    static void tick_mob(std::shared_ptr<Mobile> mob, uint32_t vec_pos);    // Processes AI for a specific active Mobile.
    static bool travel_randomly(std::shared_ptr<Mobile> mob, bool allow_dangerous_exits);   // Sends the Mobile in a random direction.
};
