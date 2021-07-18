// actions/combat.h -- Generic combat routines that apply to multiple types of combat.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_ACTIONS_COMBAT_H_
#define GREAVE_ACTIONS_COMBAT_H_

#include "world/mobile.h"

#include <cstdint>
#include <memory>
#include <string>


class Combat
{
public:
    static constexpr float  BASE_ATTACK_SPEED_MULTIPLIER =  10; // The base speed multiplier for all attacks.
    static constexpr float  STANCE_CHANGE_TIME =            1;  // The time it takes to change combat stances.

    static bool         attack(std::shared_ptr<Mobile> attacker, std::shared_ptr<Mobile> defender); // A basic attack, no special moves being used.
    static void         change_stance(std::shared_ptr<Mobile> mob, CombatStance stance);            // Changes to a specified combat stance.
    static std::string  damage_number_str(uint32_t damage, uint32_t blocked, bool crit, bool bleed, bool poison);   // Generates a standard-format damage number string.
    static std::string  damage_str(uint32_t damage, std::shared_ptr<Mobile> def, bool heat);        // Returns an appropriate damage string.

private:
    enum class WieldType : uint8_t { NONE, UNARMED, ONE_HAND_PLUS_EXTRA, TWO_HAND, DUAL_WIELD, HAND_AND_A_HALF_2H, SINGLE_WIELD, ONE_HAND_PLUS_SHIELD, SHIELD_ONLY, UNARMED_PLUS_SHIELD };

    static constexpr float  ATTACKER_DAMAGE_MULTIPLIER_ANEMIC =         0.5f;   // The damage multiplier when a Mobile with the Anemic tag attacks in melee combat.
    static constexpr float  ATTACKER_DAMAGE_MULTIPLIER_BRAWNY =         1.25f;  // The damage multiplier when a Mobile with the Brawny tag attacks in melee combat.
    static constexpr float  ATTACKER_DAMAGE_MULTIPLIER_FEEBLE =         0.75f;  // The damage multiplier when a Mobile with the Feeble tag attacks in melee combat.
    static constexpr float  ATTACKER_DAMAGE_MULTIPLIER_MIGHTY =         2.0f;   // The damage multiplier when a Mobile with the Mighty tag attacks in melee combat.
    static constexpr float  ATTACKER_DAMAGE_MULTIPLIER_PUNY =           0.9f;   // The damage multiplier when a Mobile with the Puny tag attacks in melee combat.
    static constexpr float  ATTACKER_DAMAGE_MULTIPLIER_STRONG =         1.1f;   // The damage multiplier when a Mobile with the Strong tag attacks in melee combat.
    static constexpr float  ATTACKER_DAMAGE_MULTIPLIER_VIGOROUS =       1.5f;   // The damage multiplier when a Mobile with the Vigorous tag attacks in melee combat.
    static constexpr int    BASE_ABSORPTION_VARIANCE =                  4;      // The variance in weapon damage soaked by armour (lower number = more variance).
    static constexpr float  BASE_BLOCK_CHANCE_MELEE =                   40;     // The base block chance in melee combat.
    static constexpr int    BASE_DAMAGE_VARIANCE =                      3;      // The variance in weapon damage (lower number = more variance).
    static constexpr float  BASE_HIT_CHANCE_MELEE =                     75;     // The base hit chance in melee combat.
    static constexpr float  BASE_MELEE_DAMAGE_MULTIPLIER =              1.2f;   // The base damage multiplier for melee weapons.
    static constexpr float  BASE_PARRY_CHANCE =                         10;     // The base parry chance in melee combat.
    static constexpr int    BLEED_SEVERITY_BASE =                       6;      // The base value of bleed severity, used in the bleed calculations.
    static constexpr int    BLEED_SEVERITY_RANGE =                      4;      // The range of variation on the bleed severity.
    static constexpr int    BLEED_TIME_RANGE =                          10;     // The range of time (1 - X) that a weapon bleed effect can cause.
    static constexpr float  BLOCK_SKILL_BONUS_PER_LEVEL =               0.3f;   // The bonus % chance to block per level of block skill.
    static constexpr float  CRIT_CHANCE_MULTIPLIER_SINGLE_WIELD =       1.1f;   // The multiplier to crit% for single-wielding.
    static constexpr float  DEFENDER_PARRY_MODIFIER_AGILE =             1.5f;   // The multiplier to the parry chance of a Mobile with the Agile tag.
    static constexpr float  DEFENDER_PARRY_MODIFIER_CLUMSY =            0.5f;   // The multiplier to the parry chance of a Mobile with the Clumsy tag.
    static constexpr float  DEFENDER_TO_HIT_MODIFIER_AGILE =            0.8f;   // The to-hit multiplier when attempting to hit a Mobile with the Agile tag.
    static constexpr float  DEFENDER_TO_HIT_MODIFIER_CLUMSY =           1.25f;  // The to-hit multiplier when attempting to hit a Mobile with the Clumsy tag.
    static constexpr float  EVASION_SKILL_BONUS_PER_LEVEL =             0.5f;   // The bonus % chance to dodge attacks per level of evasion skill.
    static constexpr float  HIT_CHANCE_MULTIPLIER_DUAL_WIELD =          0.9f;   // The multiplier to accuracy% for dual-wielding.
    static constexpr float  HIT_CHANCE_MULTIPLIER_SINGLE_WIELD =        1.8f;   // The multiplier to accuracy% for single-wielding.
    static constexpr float  HIT_CHANCE_MULTIPLIER_SWORD_AND_BOARD =     1.5f;   // The multiplier to accuracy% for wielding 1h+shield or 1h+extra.
    static constexpr float  PARRY_PENALTY_TWO_HANDED =                  0.6f;   // The penalty % chance to parry when using a two-handed weapon.
    static constexpr float  PARRY_SKILL_BONUS_PER_LEVEL =               0.5f;   // The bonus % chance to parry per level of parry skill.
    static constexpr int    POISON_SEVERITY_BASE =                      4;      // The base value of poison severity, used in the poison calculations.
    static constexpr int    POISON_SEVERITY_RANGE =                     6;      // The range of variation on the poison severity.
    static constexpr int    POISON_TIME_RANGE =                         5;      // The range of time (1 - X) that a weapon poison effect can cause.
    static constexpr int    SCAR_BLEED_INTENSITY_FROM_BLEED_ATTACK =    2;      // Blood type scar intensity for attacks that cause bleeding.
    static constexpr int    SCAR_BLEED_INTENSITY_FROM_DEATH =           5;      // As above, but for NPCs (which bleed) dying here.
    static constexpr float  STANCE_DAMAGE_MULTIPLIER_AGGRESSIVE =       1.2f;   // The multiplier to melee damage when in an aggressive stance.
    static constexpr float  STANCE_DAMAGE_MULTIPLIER_DEFENSIVE =        0.8f;   // The multiplier to melee damage when in a defensive stance.
    static constexpr float  STANCE_DAMAGE_TAKEN_MULTIPLIER_AGGRESSIVE = 1.2f;   // The multiplier to melee damage *taken* when in an aggressive stance.
    static constexpr float  STANCE_DAMAGE_TAKEN_MULTIPLIER_DEFENSIVE =  0.8f;   // The multiplier to melee damage *taken* when in an defensive stance.
    static constexpr float  STANCE_TO_HIT_MODIFIER_FAVOURABLE =         20;     // The to-hit % bonus when the attacker's stance is favourable vs the defender's.
    static constexpr float  STANCE_TO_HIT_MODIFIER_UNFAVOURABLE =       -10;    // The to-hit % penalty when the attacker's stance is unfavourable vs the defender's.
    static constexpr float  WEAPON_DAMAGE_MODIFIER_HAAH_2H =            1.4f;   // The damage modifier for wielding a hand-and-a-half weapon in two hands.
    static constexpr float  WEAPON_SKILL_DAMAGE_MODIFIER =              0.05f;  // The damage modifier, based on weapon skill level.
    static constexpr float  WEAPON_SKILL_TO_HIT_PER_LEVEL =             1;      // The bonus % chance to hit per point of weapon skill.
    static constexpr float  XP_PER_BLOCK =                              1;      // Experience gained for a successful shield block in combat.
    static constexpr float  XP_PER_CRITICAL_HIT =                       3;      // Weapon experience gainer per critical hit in combat.
    static constexpr float  XP_PER_EVADE =                              1;      // Experience gained for evading an attack in combat.
    static constexpr float  XP_PER_PARRY =                              1;      // Experience gained for a successful parry in combat.
    static constexpr float  XP_PER_SUCCESSFUL_HIT =                     0.7f;   // Weapon experience gained per successful weapon attack in combat.

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

#endif  // GREAVE_ACTIONS_COMBAT_H_
