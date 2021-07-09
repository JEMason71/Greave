// actions/combat/combat.hpp -- Generic combat routines that apply to multiple types of combat.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Item;                         // defined in world/item.hpp
class Mobile;                       // defined in world/mobile.hpp
enum class DamageType : int8_t;     // defined in world/item.hpp
enum class EquipSlot : uint8_t;     // defined in world/item.hpp
enum class CombatStance : uint8_t;  // defined in world/mobile.hpp


class Combat
{
public:
    static const float  BASE_ATTACK_SPEED_MULTIPLIER;   // The base speed multiplier for all attacks.
    static const float  STANCE_CHANGE_TIME;             // The time it takes to change combat stances.

    static void         change_stance(std::shared_ptr<Mobile> mob, CombatStance stance);        // Changes to a specified combat stance.
    static std::string  damage_str(uint32_t damage, std::shared_ptr<Mobile> def, bool heat);    // Returns an appropriate damage string.

protected:
    enum class WieldType : uint8_t { NONE, UNARMED, ONE_HAND_PLUS_EXTRA, TWO_HAND, DUAL_WIELD, HAND_AND_A_HALF_2H, SINGLE_WIELD, ONE_HAND_PLUS_SHIELD, SHIELD_ONLY,
        UNARMED_PLUS_SHIELD };

    static const int    BLEED_SEVERITY_BASE;                    // The base value of bleed severity, used in the bleed calculations.
    static const int    BLEED_SEVERITY_RANGE;                   // The range of variation on the bleed severity.
    static const int    BLEED_TIME_RANGE;                       // The range of time (1 - X) that a weapon bleed effect can cause.
    static const int    POISON_SEVERITY_BASE;                   // The base value of poison severity, used in the poison calculations.
    static const int    POISON_SEVERITY_RANGE;                  // The range of variation on the poison severity.
    static const int    POISON_TIME_RANGE;                      // The range of time (1 - X) that a weapon poison effect can cause.
    static const int    SCAR_BLEED_INTENSITY_FROM_BLEED_ATTACK; // Blood type scar intensity for attacks that cause bleeding.

    // Weapon type damage modifiers to unarmoured, light, medium and heavy armour targets.
    static const float DAMAGE_MODIFIER_ACID[4], DAMAGE_MODIFIER_BALLISTIC[4], DAMAGE_MODIFIER_CRUSHING[4], DAMAGE_MODIFIER_EDGED[4], DAMAGE_MODIFIER_EXPLOSIVE[4],
        DAMAGE_MODIFIER_ENERGY[4], DAMAGE_MODIFIER_KINETIC[4], DAMAGE_MODIFIER_PIERCING[4], DAMAGE_MODIFIER_PLASMA[4], DAMAGE_MODIFIER_POISON[4], DAMAGE_MODIFIER_RENDING[4];
    static const std::map<DamageType, const float*> DAMAGE_TYPE_MAP;

                        // Applies damage modifiers based on weapon type.
    static float        apply_damage_modifiers(float damage, std::shared_ptr<Item> weapon, std::shared_ptr<Mobile> defender, EquipSlot slot);
    static std::string  damage_number_str(uint32_t damage, uint32_t blocked, bool crit, bool bleed, bool poison);   // Generates a standard-format damage number string.
                        // Determines type of weapons wielded by a Mobile.
    static void         determine_wield_type(std::shared_ptr<Mobile> mob, WieldType* wield_type, bool* can_main_melee = nullptr, bool* can_off_melee = nullptr);
                        // Picks a random hit location, returns an EquipSlot and the name of the anatomy part that was hit.
    static void         pick_hit_location(std::shared_ptr<Mobile> mob, EquipSlot* slot, std::string* slot_name);
                        // Returns a threshold string, if a damage threshold has been passed.
    static int          stance_compare(CombatStance atk, CombatStance def); // Compares two combat stances; returns -1 for an unfavourable match-up, 0 for neutral, 1 for favourable.
                        // Returns a threshold string, if a damage threshold has been passed.
    static std::string  threshold_str(std::shared_ptr<Mobile> defender, uint32_t damage, const std::string& good_colour, const std::string& bad_colour);
    static void         weapon_bleed_effect(std::shared_ptr<Mobile> defender, uint32_t damage);     // Applies a weapon bleed debuff and applies room scars.
    static void         weapon_poison_effect(std::shared_ptr<Mobile> defender, uint32_t damage);    // Applies a weapon poison debuff.
};
