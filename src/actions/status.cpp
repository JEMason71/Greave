// actions/status.cpp -- Meta status actions, such as checking the player's score or condition.
/// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/status.hpp"
#include "core/core.hpp"
#include "core/strx.hpp"
#include "world/player.hpp"
#include "world/world.hpp"


// Check the player's current total score.
void ActionStatus::score()
{
    core()->message("{U}Your current score is {C}" + StrX::intostr_pretty(core()->world()->player()->score()) + "{U}.");
}
