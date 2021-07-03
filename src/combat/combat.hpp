// combat/combat.hpp -- Generic combat routines that apply to multiple types of combat.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Item;                     // defined in world/item.hpp
class Mobile;                   // defined in world/mobile.hpp
enum class DamageType : int8_t; // defined in world/item.hpp
enum class EquipSlot : uint8_t; // defined in world/item.hpp


class Combat
{
public:
    static std::string damage_str(unsigned int damage, std::shared_ptr<Mobile> def, bool heat); // Returns an appropriate damage string.

protected:
    enum class WieldType : uint8_t { NONE, UNARMED, ONE_HAND_PLUS_EXTRA, TWO_HAND, DUAL_WIELD, HAND_AND_A_HALF_2H, SINGLE_WIELD, ONE_HAND_PLUS_SHIELD, SHIELD_ONLY,
        UNARMED_PLUS_SHIELD };

    // Weapon type damage modifiers to unarmoured, light, medium and heavy armour targets.
    static const float DAMAGE_MODIFIER_ACID[4], DAMAGE_MODIFIER_BALLISTIC[4], DAMAGE_MODIFIER_CRUSHING[4], DAMAGE_MODIFIER_EDGED[4], DAMAGE_MODIFIER_EXPLOSIVE[4],
        DAMAGE_MODIFIER_ENERGY[4], DAMAGE_MODIFIER_KINETIC[4], DAMAGE_MODIFIER_PIERCING[4], DAMAGE_MODIFIER_PLASMA[4], DAMAGE_MODIFIER_POISON[4], DAMAGE_MODIFIER_RENDING[4];
    static const std::map<DamageType, const float*> DAMAGE_TYPE_MAP;

                        // Applies damage modifiers based on weapon type.
    static float        apply_damage_modifiers(float damage, std::shared_ptr<Item> weapon, std::shared_ptr<Mobile> defender, EquipSlot slot);
    static std::string  damage_number_str(int damage, int blocked, bool crit, bool bleed, bool poison); // Generates a standard-format damage number string.
                        // Determines type of weapons wielded by a Mobile.
    static void         determine_wield_type(std::shared_ptr<Mobile> mob, WieldType* wield_type, bool* can_main_melee = nullptr, bool* can_off_melee = nullptr);
                        // Picks a random hit location, returns an EquipSlot and the name of the anatomy part that was hit.
    static void         pick_hit_location(std::shared_ptr<Mobile> mob, EquipSlot* slot, std::string* slot_name);
                        // Returns a threshold string, if a damage threshold has been passed.
    static std::string  threshold_str(std::shared_ptr<Mobile> defender, int damage, const std::string& good_colour, const std::string& bad_colour);
};
