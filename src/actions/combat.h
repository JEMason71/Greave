// actions/combat.h -- Generic combat routines that apply to multiple types of combat.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once

#include <cstddef>

#include <memory>
#include <string>

#include "world/mobile.h"


class Combat
{
public:
    static const float  BASE_ATTACK_SPEED_MULTIPLIER;   // The base speed multiplier for all attacks.
    static const float  STANCE_CHANGE_TIME;             // The time it takes to change combat stances.

    static bool         attack(std::shared_ptr<Mobile> attacker, std::shared_ptr<Mobile> defender); // A basic attack, no special moves being used.
    static void         change_stance(std::shared_ptr<Mobile> mob, CombatStance stance);            // Changes to a specified combat stance.
    static std::string  damage_number_str(uint32_t damage, uint32_t blocked, bool crit, bool bleed, bool poison);   // Generates a standard-format damage number string.
    static std::string  damage_str(uint32_t damage, std::shared_ptr<Mobile> def, bool heat);        // Returns an appropriate damage string.

private:
    enum class WieldType : uint8_t { NONE, UNARMED, ONE_HAND_PLUS_EXTRA, TWO_HAND, DUAL_WIELD, HAND_AND_A_HALF_2H, SINGLE_WIELD, ONE_HAND_PLUS_SHIELD, SHIELD_ONLY, UNARMED_PLUS_SHIELD };

    static const float  ATTACKER_DAMAGE_MODIFIER_ANEMIC;            // The damage multiplier when a Mobile with the Anemic tag attacks in melee combat.
    static const float  ATTACKER_DAMAGE_MODIFIER_BRAWNY;            // The damage multiplier when a Mobile with the Brawny tag attacks in melee combat.
    static const float  ATTACKER_DAMAGE_MODIFIER_FEEBLE;            // The damage multiplier when a Mobile with the Feeble tag attacks in melee combat.
    static const float  ATTACKER_DAMAGE_MODIFIER_MIGHTY;            // The damage multiplier when a Mobile with the Mighty tag attacks in melee combat.
    static const float  ATTACKER_DAMAGE_MODIFIER_PUNY;              // The damage multiplier when a Mobile with the Puny tag attacks in melee combat.
    static const float  ATTACKER_DAMAGE_MODIFIER_STRONG;            // The damage multiplier when a Mobile with the Strong tag attacks in melee combat.
    static const float  ATTACKER_DAMAGE_MODIFIER_VIGOROUS;          // The damage multiplier when a Mobile with the Vigorous tag attacks in melee combat.
    static const float  BASE_ABSORPTION_VARIANCE;                   // The variance in weapon damage soaked by armour (lower number = more variance).
    static const float  BASE_BLOCK_CHANCE_MELEE;                    // The base block chance in melee combat.
    static const float  BASE_DAMAGE_VARIANCE;                       // The variance in weapon damage (lower number = more variance).
    static const float  BASE_HIT_CHANCE_MELEE;                      // The base hit chance in melee combat.
    static const float  BASE_MELEE_DAMAGE_MULTIPLIER;               // The base damage multiplier for melee weapons.
    static const float  BASE_PARRY_CHANCE;                          // The base parry chance in melee combat.
    static const int    BLEED_SEVERITY_BASE;                        // The base value of bleed severity, used in the bleed calculations.
    static const int    BLEED_SEVERITY_RANGE;                       // The range of variation on the bleed severity.
    static const int    BLEED_TIME_RANGE;                           // The range of time (1 - X) that a weapon bleed effect can cause.
    static const float  BLOCK_SKILL_BONUS_PER_LEVEL;                // The bonus % chance to block per level of block skill.
    static const float  CRIT_CHANCE_MULTIPLIER_SINGLE_WIELD;        // The multiplier to crit% for single-wielding.
    static const float  DEFENDER_PARRY_MODIFIER_AGILE;              // The multiplier to the parry chance of a Mobile with the Agile tag.
    static const float  DEFENDER_PARRY_MODIFIER_CLUMSY;             // The multiplier to the parry chance of a Mobile with the Clumsy tag.
    static const float  DEFENDER_TO_HIT_MODIFIER_AGILE;             // The to-hit multiplier when attempting to hit a Mobile with the Agile tag.
    static const float  DEFENDER_TO_HIT_MODIFIER_CLUMSY;            // The to-hit multiplier when attempting to hit a Mobile with the Clumsy tag.
    static const float  EVASION_SKILL_BONUS_PER_LEVEL;              // The bonus % chance to dodge attacks per level of evasion skill.
    static const float  HIT_CHANCE_MULTIPLIER_DUAL_WIELD;           // The multiplier to accuracy% for dual-wielding.
    static const float  HIT_CHANCE_MULTIPLIER_SINGLE_WIELD;         // The multiplier to accuracy% for single-wielding.
    static const float  HIT_CHANCE_MULTIPLIER_SWORD_AND_BOARD;      // The multiplier to accuracy% for wielding 1h+shield or 1h+extra.
    static const int    POISON_SEVERITY_BASE;                       // The base value of poison severity, used in the poison calculations.
    static const int    POISON_SEVERITY_RANGE;                      // The range of variation on the poison severity.
    static const int    POISON_TIME_RANGE;                          // The range of time (1 - X) that a weapon poison effect can cause.
    static const float  PARRY_PENALTY_TWO_HANDED;                   // The penalty % chance to parry when using a two-handed weapon.
    static const float  PARRY_SKILL_BONUS_PER_LEVEL;                // The bonus % chance to parry per level of parry skill.
    static const int    SCAR_BLEED_INTENSITY_FROM_BLEED_ATTACK;     // Blood type scar intensity for attacks that cause bleeding.
    static const int    SCAR_BLEED_INTENSITY_FROM_DEATH;            // As above, but for NPCs (which bleed) dying here.
    static const float  STANCE_DAMAGE_MULTIPLIER_AGGRESSIVE;        // The multiplier to melee damage when in an aggressive stance.
    static const float  STANCE_DAMAGE_MULTIPLIER_DEFENSIVE;         // The multiplier to melee damage when in a defensive stance.
    static const float  STANCE_DAMAGE_TAKEN_MULTIPLIER_AGGRESSIVE;  // The multiplier to melee damage *taken* when in an aggressive stance.
    static const float  STANCE_DAMAGE_TAKEN_MULTIPLIER_DEFENSIVE;   // The multiplier to melee damage *taken* when in an defensive stance.
    static const float  STANCE_TO_HIT_MODIFIER_FAVOURABLE;          // The to-hit % bonus when the attacker's stance is favourable vs the defender's.
    static const float  STANCE_TO_HIT_MODIFIER_UNFAVOURABLE;        // The to-hit % penalty when the attacker's stance is unfavourable vs the defender's.
    static const float  WEAPON_DAMAGE_MODIFIER_HAAH_2H;             // The damage modifier for wielding a hand-and-a-half weapon in two hands.
    static const float  WEAPON_SKILL_DAMAGE_MODIFIER;               // The damage modifier, based on weapon skill level.
    static const float  WEAPON_SKILL_TO_HIT_PER_LEVEL;              // The bonus % chance to hit per point of weapon skill.
    static const float  XP_PER_BLOCK;                               // Experience gained for a successful shield block in combat.
    static const float  XP_PER_CRITICAL_HIT;                        // Weapon experience gainer per critical hit in combat.
    static const float  XP_PER_EVADE;                               // Experience gained for evading an attack in combat.
    static const float  XP_PER_PARRY;                               // Experience gained for a successful parry in combat.
    static const float  XP_PER_SUCCESSFUL_HIT;                      // Weapon experience gained per successful weapon attack in combat.

    // Weapon type damage modifiers to unarmoured, light, medium and heavy armour targets.
    static const float DAMAGE_MODIFIER_ACID[4], DAMAGE_MODIFIER_BALLISTIC[4], DAMAGE_MODIFIER_CRUSHING[4], DAMAGE_MODIFIER_EDGED[4], DAMAGE_MODIFIER_EXPLOSIVE[4], DAMAGE_MODIFIER_ENERGY[4], DAMAGE_MODIFIER_KINETIC[4], DAMAGE_MODIFIER_PIERCING[4], DAMAGE_MODIFIER_PLASMA[4], DAMAGE_MODIFIER_POISON[4], DAMAGE_MODIFIER_RENDING[4];
    static const std::map<DamageType, const float*> DAMAGE_TYPE_MAP;

    static float        apply_damage_modifiers(float damage, std::shared_ptr<Item> weapon, std::shared_ptr<Mobile> defender, EquipSlot slot);   // Applies damage modifiers based on weapon type.
    static void         determine_wield_type(std::shared_ptr<Mobile> mob, WieldType* wield_type, bool* can_main_attack = nullptr, bool* can_off_attack = nullptr);  // Determines type of weapons wielded by a Mobile.
    static void         perform_attack(std::shared_ptr<Mobile> attacker, std::shared_ptr<Mobile> defender, EquipSlot weapon, WieldType wield_type_attacker, WieldType wield_type_defender); // Performs an attack with a single weapon.
    static void         pick_hit_location(std::shared_ptr<Mobile> mob, EquipSlot* slot, std::string* slot_name);    // Picks a random hit location, returns an EquipSlot and the name of the anatomy part that was hit.
    static int          stance_compare(CombatStance atk, CombatStance def); // Compares two combat stances; returns -1 for an unfavourable match-up, 0 for neutral, 1 for favourable.
    static std::string  threshold_str(std::shared_ptr<Mobile> defender, uint32_t damage, const std::string& good_colour, const std::string& bad_colour);    // Returns a threshold string, if a damage threshold has been passed.
    static void         weapon_bleed_effect(std::shared_ptr<Mobile> defender, uint32_t damage);     // Applies a weapon bleed debuff and applies room scars.
    static void         weapon_poison_effect(std::shared_ptr<Mobile> defender, uint32_t damage);    // Applies a weapon poison debuff.
};
