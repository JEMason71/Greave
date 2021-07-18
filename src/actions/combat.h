// actions/combat.h -- Generic combat routines that apply to multiple types of combat.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_ACTIONS_COMBAT_H_
#define GREAVE_ACTIONS_COMBAT_H_

#include <cstddef>

#include <memory>
#include <string>

#include "world/mobile.h"


namespace greave
{

enum class WieldType : uint8_t { NONE, UNARMED, ONE_HAND_PLUS_EXTRA, TWO_HAND, DUAL_WIELD, HAND_AND_A_HALF_2H, SINGLE_WIELD, ONE_HAND_PLUS_SHIELD, SHIELD_ONLY, UNARMED_PLUS_SHIELD };

namespace combat
{

constexpr float kStanceChangeTime = 1;  // The time it takes to change combat stances.

float       apply_damage_modifiers(float damage, std::shared_ptr<Item> weapon, std::shared_ptr<Mobile> defender, EquipSlot slot);   // Applies damage modifiers based on weapon type.
bool        attack(std::shared_ptr<Mobile> attacker, std::shared_ptr<Mobile> defender); // A basic attack, no special moves being used.
void        change_stance(std::shared_ptr<Mobile> mob, CombatStance stance);            // Changes to a specified combat stance.
std::string damage_number_str(uint32_t damage, uint32_t blocked, bool crit, bool bleed, bool poison);   // Generates a standard-format damage number string.
std::string damage_str(uint32_t damage, std::shared_ptr<Mobile> def, bool heat);        // Returns an appropriate damage string.
void        determine_wield_type(std::shared_ptr<Mobile> mob, WieldType* wield_type, bool* can_main_attack = nullptr, bool* can_off_attack = nullptr); // Determines type of weapons wielded by a Mobile.
void        perform_attack(std::shared_ptr<Mobile> attacker, std::shared_ptr<Mobile> defender, EquipSlot weapon, WieldType wield_type_attacker, WieldType wield_type_defender); // Performs an attack with a single weapon.
void        pick_hit_location(std::shared_ptr<Mobile> mob, EquipSlot* slot, std::string* slot_name);    // Picks a random hit location, returns an EquipSlot and the name of the anatomy part that was hit.
int         stance_compare(CombatStance atk, CombatStance def); // Compares two combat stances; returns -1 for an unfavourable match-up, 0 for neutral, 1 for favourable.
std::string threshold_str(std::shared_ptr<Mobile> defender, uint32_t damage, const std::string& good_colour, const std::string& bad_colour);    // Returns a threshold string, if a damage threshold has been passed.
void        weapon_bleed_effect(std::shared_ptr<Mobile> defender, uint32_t damage);     // Applies a weapon bleed debuff and applies room scars.
void        weapon_poison_effect(std::shared_ptr<Mobile> defender, uint32_t damage);    // Applies a weapon poison debuff.

}       // namespace combat
}       // namespace greave
#endif  // GREAVE_ACTIONS_COMBAT_H_
