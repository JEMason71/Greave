// actions/abilities.h -- Special abilities which can be used in combat.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_ACTIONS_ABILITIES_H_
#define GREAVE_ACTIONS_ABILITIES_H_

#include <cstddef>


namespace greave {
namespace abilities {

void abilities();                                   // Check cooldowns and availability of abilities.
void careful_aim(bool confirm);                     // Attempt to use the Careful Aim ability.
void eye_for_an_eye(bool confirm);                  // Attempt to use the Eye for an Eye ability.
void grit(bool confirm);                            // Attempt to use the Grit ability.
void headlong_strike(size_t target, bool confirm);  // Attempt to use the HeadlongStrike ability.
void lady_luck(size_t target, bool confirm);        // Attempt to use the Lady Luck ability.
void quick_roll(bool confirm);                      // Attempt to use the Quick Roll ability.
void rapid_strike(size_t target);                   // Attempt to use the Rapid Strike ability.
void shield_wall(bool confirm);                     // Attempt to use the Shield Wall ability.
void snap_shot(size_t target);                      // Attempt to use the Snap Shot ability.

}       // namespace abilities
}       // namespace greave
#endif  // GREAVE_ACTIONS_ABILITIES_H_
