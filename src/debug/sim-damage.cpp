// debug/sim-damage.cpp -- Very simple simulation for different weapon types against armour.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "core/core.hpp"
#include "core/random.hpp"
#include "core/strx.hpp"
#include "debug/sim-damage.hpp"


// Constructor, sets blank default values.
DamageSim::DamageSim() : m_armour_percent(0), m_armour_roll_dice(0), m_armour_roll_die_faces(0), m_armour_roll_modifier(0), m_rng(std::make_shared<Random>()) { }

// Armour damage simulation.
void DamageSim::sim_armour(uint8_t armour_percent, uint8_t armour_roll_dice, uint8_t armour_roll_die_faces, int8_t armour_roll_modifier)
{
    m_armour_percent = armour_percent;
    m_armour_roll_dice = armour_roll_dice;
    m_armour_roll_die_faces = armour_roll_die_faces;
    m_armour_roll_modifier = armour_roll_modifier;

    core()->message("{c}Running simulation for armour with {C}" + std::to_string(armour_percent) + "{c}% chance of {C}" + std::to_string(m_armour_roll_dice) + "{c}d{C}" +
        std::to_string(m_armour_roll_die_faces) + "{c}" + (m_armour_roll_modifier >= 0 ? "+" : "-") + "{C}" + std::to_string(std::abs(m_armour_roll_modifier)) +
        " {c}damage reduction.");
    sim_armour_individual("dagger", 1, 4);
    sim_armour_individual("shortsword", 1, 6);
    sim_armour_individual("longsword", 1, 8);
    sim_armour_individual("halberd", 1, 10);
    sim_armour_individual("lance", 1, 12);
    sim_armour_individual("greatsword", 2, 6);
}

// Single damage simulation of one weapon type.
void DamageSim::sim_armour_individual(const std::string &weapon_name, uint8_t weapon_roll_dice, uint8_t weapon_roll_die_faces, int8_t weapon_roll_modifier)
{
    const int iterations = 1000000; // The higher this number, the more accurate yet slower the simulations will be.
    int total_raw_damage = 0, total_adjusted_damage = 0;
    for (int i = 0; i < iterations; i++)
    {
        const int weapon_damage = m_rng->roll(weapon_roll_dice, weapon_roll_die_faces, weapon_roll_modifier);
        total_raw_damage += weapon_damage;
        int armour_reduction = 0;
        if (m_rng->percent_check(m_armour_percent)) armour_reduction = m_rng->roll(m_armour_roll_dice, m_armour_roll_die_faces, m_armour_roll_modifier);
        if (armour_reduction < weapon_damage) total_adjusted_damage += (weapon_damage - armour_reduction);
    }
    const float damage_percent = (static_cast<float>(total_adjusted_damage) / static_cast<float>(total_raw_damage)) * 100.0f;
    const float average_raw_damage = (static_cast<float>(total_raw_damage) / static_cast<float>(iterations));
    const float average_adjusted_damage = (static_cast<float>(total_adjusted_damage) / static_cast<float>(iterations));
    core()->message("{0}{G}" + std::to_string(weapon_roll_dice) + "{g}d{G}" + std::to_string(weapon_roll_die_faces) + "{g}" + (weapon_roll_modifier >= 0 ? "+" : "-") +
        "{G}" + std::to_string(std::abs(weapon_roll_modifier)) + " {g}" + weapon_name + ": {Y}" + StrX::ftos(damage_percent) + "{y}% average (~{Y}" +
        StrX::round_to_two(average_raw_damage) + "{y} raw, ~{Y}" + StrX::round_to_two(average_adjusted_damage) + " {y}adjusted)");
}
