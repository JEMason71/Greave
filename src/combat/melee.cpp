// combat/melee.cpp -- The melee combat engine, handles melee-specific combat between Mobiles.
// Copyright (c) 2021 Raine "Gravecat" Simmons. All rights reserved.

#include "combat/melee.hpp"
#include "core/core.hpp"
#include "core/random.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/player.hpp"
#include "world/room.hpp"
#include "world/world.hpp"


const float Melee::BASE_HIT_CHANCE_MELEE =                  75.0f;  // The base hit chance in melee combat.
const float Melee::DUAL_WIELD_HIT_CHANCE_MULTIPLIER =       0.9f;   // The multiplier to accuracy% for dual-wielding.
const float Melee::SINGLE_WIELD_HIT_CHANCE_MULTIPLIER =     1.2f;   // The multiplier to accuracy% for single-wielding.
const float Melee::SWORD_AND_BOARD_HIT_CHANCE_MULTIPLIER =  1.1f;   // The multiplier to accuracy% for wielding 1h+shield or 1h+extra.
const float Melee::WEAPON_DAMAGE_MODIFIER_HAAH_2H =         1.8f;   // The damage modifier for wielding a hand-and-a-half weapon in two hands.


// A basic attack, no special moves being used.
bool Melee::attack(std::shared_ptr<Mobile> attacker, std::shared_ptr<Mobile> defender)
{
    if (attacker->is_dead() || defender->is_dead()) return false;

    // We'll need to check the equipment for both attacker and defender, so we can determine how both are wielding their weapons.
    std::shared_ptr<Item> main_hand[2], off_hand[2];
    bool main_can_attack[2], off_can_attack[2];
    WieldType wield_type[2];

    for (int i = 0; i < 2; i++)
    {
        std::shared_ptr<Mobile> mob = (i == 0 ? attacker : defender);
        determine_wield_type(mob, &wield_type[i], &main_can_attack[i], &off_can_attack[i]);
        if (i == 0 && wield_type[i] == WieldType::NONE) return false;   // Just give up here if the attacker can't attack.
    }
    if (!main_can_attack[0] && !off_can_attack[0]) return false;        // Should be impossible, but can't hurt to be safe.

    const bool unarmed_only = (wield_type[0] == WieldType::UNARMED || wield_type[0] == WieldType::UNARMED_PLUS_SHIELD);
    float attack_speed = attacker->attack_speed();

    bool attacked = false;
    if (main_can_attack[0])
    {
        perform_attack(attacker, defender, EquipSlot::HAND_MAIN, wield_type[0], wield_type[1]);
        attacked = true;
    }
    if (off_can_attack[0] && !attacker->is_dead() && !defender->is_dead() && !unarmed_only)
    {
        perform_attack(attacker, defender, EquipSlot::HAND_OFF, wield_type[0], wield_type[1]);
        attacked = true;
    }
    attacker->pass_time(attack_speed);
    return attacked;
}

// Performs an attack with a single weapon.
void Melee::perform_attack(std::shared_ptr<Mobile> attacker, std::shared_ptr<Mobile> defender, EquipSlot weapon, WieldType wield_type_attacker, WieldType wield_type_defender)
{
    std::shared_ptr<Item> weapon_ptr = attacker->equ()->get(weapon);
    if (!weapon_ptr) weapon_ptr = core()->world()->get_item("UNARMED_ATTACK");
    const std::shared_ptr<Item> def_weapon_main = defender->equ()->get(EquipSlot::HAND_MAIN);
    const std::shared_ptr<Item> def_weapon_off = defender->equ()->get(EquipSlot::HAND_OFF);

    const bool attacker_is_player = attacker->is_player();
    const bool defender_is_player = defender->is_player();
    const bool player_is_here = (core()->world()->player()->location() == attacker->location());
    const bool is_dark_here = core()->world()->get_room(attacker->location())->light(core()->world()->player()) < Room::LIGHT_VISIBLE;
    const bool player_can_see_attacker = attacker_is_player || !is_dark_here;
    const bool player_can_see_defender = defender_is_player || !is_dark_here;
    const bool defender_melee = (wield_type_defender != WieldType::UNARMED && wield_type_defender != WieldType::UNARMED_PLUS_SHIELD) &&
        ((def_weapon_main && def_weapon_main->type() == ItemType::WEAPON && def_weapon_main->subtype() == ItemSub::MELEE) ||
        (def_weapon_off && def_weapon_off->type() == ItemType::WEAPON && def_weapon_off->subtype() == ItemSub::MELEE));

    const std::string attacker_name = (attacker_is_player ? "you" : (player_can_see_attacker ? attacker->name(Mobile::NAME_FLAG_THE) : "something"));
    const std::string defender_name = (defender_is_player ? "you" : (player_can_see_defender ? defender->name(Mobile::NAME_FLAG_THE) : "something"));
    const std::string defender_name_c = StrX::capitalize_first_letter(defender_name);
    const std::string defender_name_s = (defender_is_player ? "your" : StrX::possessive_string(defender_name));
    const std::string defender_your_string = (defender_is_player ? "your" : defender->his_her());
    const std::string defender_your_string_c = StrX::capitalize_first_letter(defender_your_string);
    const std::string attacker_your_string = (attacker_is_player ? "your" : StrX::possessive_string(attacker_name));
    const std::string attacker_your_string_c = StrX::capitalize_first_letter(attacker_your_string);
    const std::string weapon_name = weapon_ptr->name();

    // Roll to hit!
    float hit_multiplier = 1.0f;
    switch (wield_type_attacker)
    {
        case WieldType::DUAL_WIELD: hit_multiplier = DUAL_WIELD_HIT_CHANCE_MULTIPLIER; break;
        case WieldType::SINGLE_WIELD: hit_multiplier = SINGLE_WIELD_HIT_CHANCE_MULTIPLIER; break;
        case WieldType::ONE_HAND_PLUS_SHIELD: case WieldType::UNARMED_PLUS_SHIELD: case WieldType::ONE_HAND_PLUS_EXTRA:
            hit_multiplier = SWORD_AND_BOARD_HIT_CHANCE_MULTIPLIER; break;
        default: break;
    }
    float to_hit = BASE_HIT_CHANCE_MELEE * hit_multiplier;
    
    bool evaded = false, blocked = false;
    if (core()->rng()->frnd(100) <= to_hit)  // Evasion failed; the target was hit.
    {
        // parry and block code goes here.
    }
    else evaded = true;

    // Determine where the defender was hit.
    EquipSlot def_location_hit_es;
    std::string def_location_hit_str;
    pick_hit_location(defender, &def_location_hit_es, &def_location_hit_str);

    //if (parried || evaded)
    if (evaded)
    {
        //if (parried) { }
        //else
        {
            if (player_can_see_attacker || player_can_see_defender)
                core()->message((attacker_is_player ? "{Y}" : (defender_is_player ? "{U}" : "{U}")) + attacker_your_string_c + " " + weapon_name + " misses " + defender_name + ".");
        }
    }
    else
    {
        float damage = weapon_ptr->power();
        if (wield_type_attacker == WieldType::HAND_AND_A_HALF_2H) damage *= WEAPON_DAMAGE_MODIFIER_HAAH_2H;

        bool critical_hit = false, bleed = false, poison = false;

        std::shared_ptr<Item> armour_piece_hit = defender->equ()->get(def_location_hit_es);
        float damage_blocked = 0;

        if (damage > 1) damage = std::round(damage);
        else if (damage > 0) damage = 1;
        if (damage_blocked > 1) damage_blocked = std::round(damage_blocked);
        else if (damage_blocked > 0) damage_blocked = 1;
        if (damage_blocked >= damage) damage_blocked = damage;
        damage -= damage_blocked;

        if (player_is_here && (player_can_see_attacker || player_can_see_defender))
        {
            std::string damage_word = damage_str(damage, defender, false);
            std::string threshold_string = threshold_str(defender, damage, (attacker_is_player ? "{G}" : (defender_is_player ? "{R}" : "{U}")), (defender_is_player ? "{Y}" :
                (attacker_is_player ? "{y}" : "{U}")));
            std::string damage_colour = (attacker_is_player ? (damage > 0.0f ? "{G}" : "{y}") : (defender_is_player ? (damage > 0.0f ? "{R}" : "{Y}") : "{U}"));
            std::string absorb_str, block_str, death_str;

            if (damage >= defender->hp())
            {
                if (defender_is_player) death_str = " {M}You are slain!";
                else death_str = " {U}" + defender_name_c + (defender->tag(MobileTag::Unliving) ? " is destroyed!" : " is slain!");
            }
            core()->message(block_str + damage_colour + attacker_your_string_c + " " + weapon_name + " " + damage_word + " " + damage_colour +
                (blocked ? defender_name : defender_name_s + " " + def_location_hit_str) + "!" + threshold_string + absorb_str + " " +
                damage_number_str(damage, damage_blocked, critical_hit, bleed, poison) + death_str);
            defender->reduce_hp(damage);
        }
    }
}
