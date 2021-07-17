// actions/ai.h -- NPC AI actions and behaviour.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_ACTIONS_AI_H_
#define GREAVE_ACTIONS_AI_H_

#include <memory>

#include "world/mobile.h"


namespace greave {
namespace mobile_ai {

void    tick_mob(std::shared_ptr<Mobile> mob, uint32_t vec_pos);                    // Processes AI for a specific active Mobile.
void    tick_mobs();                                                                // Ticks all the mobiles in active rooms.
bool    travel_randomly(std::shared_ptr<Mobile> mob, bool allow_dangerous_exits);   // Sends the Mobile in a random direction.

}       // namespace mobile_ai
}       // namespace greave
#endif  // GREAVE_ACTIONS_AI_H_
