// world/ai.cpp -- NPC AI actions and behaviour.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/travel.hpp"
#include "core/core.hpp"
#include "core/random.hpp"
#include "world/ai.hpp"
#include "world/mobile.hpp"
#include "world/room.hpp"
#include "world/world.hpp"


const uint32_t  AI::TRAVEL_CHANCE = 300;    // 1 in X chance of traveling to another room.


// Processes AI for a specific active Mobile.
void AI::tick_mob(std::shared_ptr<Mobile> mob, uint32_t)
{
    auto rng = core()->rng();
    auto room = core()->world()->get_room(mob->location());

    if (rng->rnd(TRAVEL_CHANCE) == 1)
    {
        std::vector<uint8_t> viable_exits;
        for (unsigned int i = 0; i < Room::ROOM_LINKS_MAX; i++)
        {
            if (room->fake_link(i)) continue;
            if (room->dangerous_link(i)) continue;
            if (room->link_tag(i, LinkTag::Locked)) continue;
            viable_exits.push_back(i);
        }
        if (viable_exits.size())
        {
            ActionTravel::travel(mob, static_cast<Direction>(viable_exits.at(rng->rnd(0, viable_exits.size() - 1))), true);
            return;
        }
    }
}

// Ticks all the mobiles in active rooms.
void AI::tick_mobs()
{
    for (unsigned int m = 0; m < core()->world()->mob_count(); m++)
    {
        const auto mob = core()->world()->mob_vec(m);
        mob->restore_action_timer(1.0f);    // tick_mobs() is called every in-game second.
        if (!mob->can_act()) continue;
        tick_mob(mob, m);
    }
}
