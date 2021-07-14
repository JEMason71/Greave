// actions/abilities.hpp -- Special abilities which can be used in combat.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


class Abilities
{
public:
    static void abilities();                // Check cooldowns and availability of abilities.
    static void careful_aim(bool confirm);  // Attempt to use the Careful Aim ability.

private:
    static float    CAREFUL_AIM_BONUS_HIT;  // The bonus hit% chance from using the Careful Aim ability.
    static int      CAREFUL_AIM_COOLDOWN;   // The length of the Careful Aim cooldown.
    static int      CAREFUL_AIM_LENGTH;     // How many buff ticks the Careful Aim ability lasts for.
    static float    CAREFUL_AIM_TIME;       // The time taken by the Careful Aim ability.
    static int      MP_COST_CAREFUL_AIM;    // The mana point cost for the Careful Aim ability.
};
