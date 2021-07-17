// actions/arena.h -- The arena, where you can participate in fights or bet on NPC fights.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_ACTIONS_ARENA_H_
#define GREAVE_ACTIONS_ARENA_H_


namespace greave {
namespace arena {

void    combatant_died();   // One of the arena combatant mobiles died.
void    fight_over();       // The battle has been won!
void    participate();      // Participates in an arena fight!
void    reward();           // Time to collect your fight money.

}       // namespace arena
}       // namespace greave
#endif  // GREAVE_ACTIONS_ARENA_H_
