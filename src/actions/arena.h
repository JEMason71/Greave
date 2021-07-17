// actions/arena.h -- The arena, where you can participate in fights or bet on NPC fights.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once


class Arena
{
public:
    static void combatant_died();   // One of the arena combatant mobiles died.
    static void participate();      // Participates in an arena fight!
    static void reward();           // Time to collect your fight money.

private:
    static void fight_over();       // The battle has been won!
};
