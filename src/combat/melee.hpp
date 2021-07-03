// combat/melee.hpp -- The melee combat engine, handles melee-specific combat between Mobiles.
// Copyright (c) 2021 Raine "Gravecat" Simmons. All rights reserved.

#pragma once
#include "combat/combat.hpp"

class Mobile;                   // defined in world/mobile.hpp
enum class EquipSlot : uint8_t; // defined in world/item.hpp


class Melee : public Combat
{
public:
    static bool attack(std::shared_ptr<Mobile> attacker, std::shared_ptr<Mobile> defender); // A basic attack, no special moves being used.

private:
    static const float  BASE_BLOCK_CHANCE_MELEE;                // The base block chance in melee combat.
    static const float  BASE_HIT_CHANCE_MELEE;                  // The base hit chance in melee combat.
    static const float  BASE_PARRY_CHANCE;                      // The base parry chance in melee combat.
    static const float  DUAL_WIELD_HIT_CHANCE_MULTIPLIER;       // The multiplier to accuracy% for dual-wielding.
    static const float  SINGLE_WIELD_CRIT_CHANCE_MULTIPLIER;    // The multiplier to crit% for single-wielding.
    static const float  SINGLE_WIELD_HIT_CHANCE_MULTIPLIER;     // The multiplier to accuracy% for single-wielding.
    static const float  SWORD_AND_BOARD_HIT_CHANCE_MULTIPLIER;  // The multiplier to accuracy% for wielding 1h+shield or 1h+extra.
    static const float  WEAPON_DAMAGE_MODIFIER_HAAH_2H;         // The damage modifier for wielding a hand-and-a-half weapon in two hands.

                // Performs an attack with a single weapon.
    static void perform_attack(std::shared_ptr<Mobile> attacker, std::shared_ptr<Mobile> defender, EquipSlot weapon, WieldType wield_type_attacker, WieldType wield_type_defender);
};
