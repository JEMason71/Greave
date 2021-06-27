// debug/sim-damage.hpp -- Very simple simulation for different weapon types against armour.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Random;   // defined in core/random.hpp


class DamageSim
{
public:
            DamageSim();    // Constructor, sets blank default values.
    void    sim_armour(uint8_t armour_percent, uint8_t armour_roll_dice, uint8_t armour_roll_die_faces, uint8_t armour_roll_modifier = 0);  // Armour damage simulation.

private:
    // Single damage simulation of one weapon type.
    void    sim_armour_individual(const std::string &weapon_name, uint8_t weapon_roll_dice, uint8_t weapon_roll_die_faces, uint8_t weapon_roll_modifier = 0);

    uint8_t m_armour_percent;           // The % damage reduction of simulated armour.
    uint8_t m_armour_roll_dice;         // The number of dice used in simulated armour damage reduction.
    uint8_t m_armour_roll_die_faces;    // The number of faces used on each die for simulated armour damage reduction.
    uint8_t m_armour_roll_modifier;     // The +/- modifier (if any) to damage reduction on simulated armour.
    std::shared_ptr<Random> m_rng;      // The random number generator.
};
