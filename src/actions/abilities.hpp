// actions/abilities.hpp -- Special abilities which can be used in combat.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


class Abilities
{
public:
    static float    RAPID_STRIKE_ACCURACY_PENALTY;          // The % accuracy penalty for a Rapid Strike.
    static float    RAPID_STRIKE_ATTACK_SPEED;              // The % of an attack's normal speed that it takes to do a Rapid Strike attack.
    static float    SNAP_SHOT_ACCURACY_PENALTY;             // The % accuracy penalty for a Snap Shot.
    static float    SNAP_SHOT_ATTACK_SPEED;                 // The % of an attack's normal speed that it takes to do a Snap Shot attack.

    static void abilities();                                // Check cooldowns and availability of abilities.
    static void careful_aim(bool confirm);                  // Attempt to use the Careful Aim ability.
    static void eye_for_an_eye(bool confirm);               // Attempt to use the Eye for an Eye ability.
    static void grit(bool confirm);                         // Attempt to use the Grit ability.
    static void lady_luck(size_t target, bool confirm);     // Attempt to use the Lady Luck ability.
    static void quick_roll(bool confirm);                   // Attempt to use the Quick Roll ability.
    static void rapid_strike(size_t target);                // Attempt to use the Rapid Strike ability.
    static void snap_shot(size_t target);                   // Attempt to use the Snap Shot ability.

private:
    static float    CAREFUL_AIM_BONUS_HIT;      // The bonus hit% chance from using the Careful Aim ability.
    static int      CAREFUL_AIM_COOLDOWN;       // The length of the Careful Aim cooldown.
    static int      CAREFUL_AIM_LENGTH;         // How many buff ticks the Careful Aim ability lasts for.
    static int      CAREFUL_AIM_MP_COST;        // The mana point cost for the Careful Aim ability.
    static float    CAREFUL_AIM_TIME;           // The time taken by the Careful Aim ability.
    static int      EYE_FOR_AN_EYE_COOLDOWN;    // The cooldown for the Eye For An Eye ability.
    static int      EYE_FOR_AN_EYE_HP_COST;     // The hit points cost for using Eye for an Eye.
    static int      EYE_FOR_AN_EYE_LENGTH;      // The length of time the Eye For An Eye buff remains when activated but unused.
    static float    EYE_FOR_AN_EYE_MULTI;       // The damage multiplier for the Eye For An Eye ability.
    static int      GRIT_COOLDOWN;              // The cooldown for the Grit ability.
    static float    GRIT_DAMAGE_REDUCTION;      // The % of damage reduced by using the Grit ability.
    static int      GRIT_LENGTH;                // The Grit ability lasts this long, or until the player is hit by an attack.
    static int      GRIT_SP_COST;               // The stamina point cost for the Grit ability.
    static float    GRIT_TIME;                  // The time taken by using the Grit ability.
    static int      LADY_LUCK_COOLDOWN;         // The cooldown for the Lady Luck ability.
    static int      LADY_LUCK_LENGTH;           // The buff/debuff time for the Lady Luck ability.
    static int      LADY_LUCK_MP_COST;          // The mana cost for using the Lady Luck ability.
    static float    LADY_LUCK_TIME;             // The time taken by using the Lady Luck ability.
    static int      QUICK_ROLL_BONUS_DODGE;     // The bonus dodge% chance from using the Quick Roll ability.
    static int      QUICK_ROLL_COOLDOWN;        // The cooldown for the Quick Roll ability.
    static int      QUICK_ROLL_LENGTH;          // The length of time the Quick Roll buff remains when activated, but before an enemy attack is made.
    static int      QUICK_ROLL_SP_COST;         // The stamina point cost for the Quick Roll ability.
    static int      QUICK_ROLL_TIME;            // The time it takes to do a Quick Roll.
    static int      RAPID_STRIKE_COOLDOWN;      // The cooldown for the Rapid Strike ability.
    static int      RAPID_STRIKE_SP_COST;       // The stamina points cost for the Rapid Strike ability.
    static int      SNAP_SHOT_COOLDOWN;         // The cooldown for the Snap Shot ability.
    static int      SNAP_SHOT_SP_COST;          // The stamina points cost for the Snap Shot ability.
};
