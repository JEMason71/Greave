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
    static const uint32_t   TRAVEL_CHANCE;  // 1 in X chance of traveling to another room.

    static void tick_mob(std::shared_ptr<Mobile> mob, uint32_t vec_pos);    // Processes AI for a specific active Mobile.
};
