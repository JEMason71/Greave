// actions/abilities.cpp -- Special abilities which can be used in combat.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/abilities.hpp"
#include "core/core.hpp"


// Check cooldowns and availability of abilities.
void Abilities::abilities()
{
    core()->message("{m}You have no special abilities available.");
}
