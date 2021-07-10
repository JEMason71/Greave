// actions/combat/melee.hpp -- The melee combat engine, handles melee-specific combat between Mobiles.
// Copyright (c) 2021 Raine "Gravecat" Simmons. All rights reserved.

#pragma once
#include "actions/combat/combat.hpp"

class Mobile;                   // defined in world/mobile.hpp
enum class EquipSlot : uint8_t; // defined in world/item.hpp


class Melee : public Combat
{
public:
    static bool attack(std::shared_ptr<Mobile> attacker, std::shared_ptr<Mobile> defender); // A basic attack, no special moves being used.

private:
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
    static const float  PARRY_SKILL_BONUS_PER_LEVEL;                // The bonus % chance to parry per level of parry skill.
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

                // Performs an attack with a single weapon.
    static void perform_attack(std::shared_ptr<Mobile> attacker, std::shared_ptr<Mobile> defender, EquipSlot weapon, WieldType wield_type_attacker, WieldType wield_type_defender);
};
