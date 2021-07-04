// actions/ai.cpp -- NPC AI actions and behaviour.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/ai.hpp"
#include "actions/travel.hpp"
#include "combat/melee.hpp"
#include "core/core.hpp"
#include "core/random.hpp"
#include "world/player.hpp"
#include "world/room.hpp"
#include "world/world.hpp"


const uint32_t  AI::AGGRO_CHANCE =  60;     // 1 in X chance of starting a fight.
const uint32_t  AI::TRAVEL_CHANCE = 300;    // 1 in X chance of traveling to another room.


// Processes AI for a specific active Mobile.
void AI::tick_mob(std::shared_ptr<Mobile> mob, uint32_t)
{
    auto rng = core()->rng();
    const uint32_t location = mob->location();
    const uint32_t player_location = core()->world()->player()->location();
    auto room = core()->world()->get_room(location);

    // Scan the Mobile's hostility vector, looking for anyone they're hostile towards.
    std::shared_ptr<Mobile> attack_target = nullptr;
    for (auto h : mob->hostility_vector())
    {
        if (h == 0 && mob->location() == core()->world()->player()->location())
        {
            attack_target = core()->world()->player();
            break;
        }
        else for (unsigned int m = 0; m < core()->world()->mob_count(); m++)
        {
            const auto check_mob = core()->world()->mob_vec(m);
            if (check_mob->id() == h && check_mob->location() == mob->location())
            {
                attack_target = check_mob;
                break;
            }
        }
    }
    if (attack_target)
    {
        if (mob->tag(MobileTag::Coward))
        {
            // Attempt a safe travel; if it fails, panic and attempt a more dangerous exit.
            if (location == player_location) core()->message("{U}" + mob->name(Mobile::NAME_FLAG_THE) + " {U}flees in a blind panic!", Show::ACTIVE, Wake::NEVER);
            if (!travel_randomly(mob, true))
            {
                mob->pass_time(600);
                if (location == player_location) core()->message("{0}{u}... But " + mob->he_she() + " can't get away!", Show::ACTIVE, Wake::NEVER);
            }
        }
        else Melee::attack(mob, attack_target);
        return;
    }

    if (mob->tag(MobileTag::AggroOnSight) && rng->rnd(AGGRO_CHANCE) == 1 && location == player_location)
    {
        Melee::attack(mob, core()->world()->player());
        return;
    }

    if (rng->rnd(TRAVEL_CHANCE) == 1 && travel_randomly(mob, false)) return;
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

// Sends the Mobile in a random direction.
bool AI::travel_randomly(std::shared_ptr<Mobile> mob, bool allow_dangerous_exits)
{
    const uint32_t location = mob->location();
    auto room = core()->world()->get_room(location);

    std::vector<uint8_t> viable_exits;
    for (unsigned int i = 0; i < Room::ROOM_LINKS_MAX; i++)
    {
        if (room->fake_link(i)) continue;
        if (!allow_dangerous_exits && room->dangerous_link(i)) continue;
        if (room->link_tag(i, LinkTag::Locked)) continue;
        viable_exits.push_back(i);
    }
    if (viable_exits.size()) return ActionTravel::travel(mob, static_cast<Direction>(viable_exits.at(core()->rng()->rnd(0, viable_exits.size() - 1))), true);
    else return false;
}
