// combat/melee.cpp -- The melee combat engine, handles melee-specific combat between Mobiles.
// Copyright (c) 2021 Raine "Gravecat" Simmons. All rights reserved.

#include "combat/melee.hpp"
#include "core/core.hpp"
#include "core/mathx.hpp"
#include "core/random.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/player.hpp"
#include "world/room.hpp"
#include "world/world.hpp"


const float Melee::ATTACKER_DAMAGE_MODIFIER_ANEMIC =        0.5f;   // The damage multiplier when a Mobile with the Anemic tag attacks in melee combat.
const float Melee::ATTACKER_DAMAGE_MODIFIER_BRAWNY =        1.25f;  // The damage multiplier when a Mobile with the Brawny tag attacks in melee combat.
const float Melee::ATTACKER_DAMAGE_MODIFIER_FEEBLE =        0.75f;  // The damage multiplier when a Mobile with the Feeble tag attacks in melee combat.
const float Melee::ATTACKER_DAMAGE_MODIFIER_MIGHTY =        2.0f;   // The damage multiplier when a Mobile with the Mighty tag attacks in melee combat.
const float Melee::ATTACKER_DAMAGE_MODIFIER_PUNY =          0.9f;   // The damage multiplier when a Mobile with the Puny tag attacks in melee combat.
const float Melee::ATTACKER_DAMAGE_MODIFIER_STRONG =        1.1f;   // The damage multiplier when a Mobile with the Strong tag attacks in melee combat.
const float Melee::ATTACKER_DAMAGE_MODIFIER_VIGOROUS =      1.5f;   // The damage multiplier when a Mobile with the Vigorous tag attacks in melee combat.
const float Melee::BASE_ABSORPTION_VARIANCE =               4;      // The variance in weapon damage soaked by armour (lower number = more variance).
const float Melee::BASE_BLOCK_CHANCE_MELEE =                20.0f;  // The base block chance in melee combat.
const float Melee::BASE_DAMAGE_VARIANCE =                   3;      // The variance in weapon damage (lower number = more variance).
const float Melee::BASE_HIT_CHANCE_MELEE =                  75.0f;  // The base hit chance in melee combat.
const float Melee::BASE_MELEE_DAMAGE_MULTIPLIER =           1.2f;   // The base damage multiplier for melee weapons.
const float Melee::BASE_PARRY_CHANCE =                      10.0f;  // The base parry chance in melee combat.
const float Melee::CRIT_CHANCE_MULTIPLIER_SINGLE_WIELD =    1.1f;   // The multiplier to crit% for single-wielding.
const float Melee::DEFENDER_PARRY_MODIFIER_AGILE =          1.5f;   // The multiplier to the parry chance of a Mobile with the Agile tag.
const float Melee::DEFENDER_PARRY_MODIFIER_CLUMSY =         0.5f;   // The multiplier to the parry chance of a Mobile with the Clumsy tag.
const float Melee::DEFENDER_TO_HIT_MODIFIER_AGILE =         0.8f;   // The to-hit multiplier when attempting to hit a Mobile with the Agile tag.
const float Melee::DEFENDER_TO_HIT_MODIFIER_CLUMSY =        1.25f;  // The to-hit multiplier when attempting to hit a Mobile with the Clumsy tag.
const float Melee::HIT_CHANCE_MULTIPLIER_DUAL_WIELD =       0.9f;   // The multiplier to accuracy% for dual-wielding.
const float Melee::HIT_CHANCE_MULTIPLIER_SINGLE_WIELD =     1.2f;   // The multiplier to accuracy% for single-wielding.
const float Melee::HIT_CHANCE_MULTIPLIER_SWORD_AND_BOARD =  1.1f;   // The multiplier to accuracy% for wielding 1h+shield or 1h+extra.
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
    const float attack_speed = attacker->attack_speed() * BASE_ATTACK_SPEED_MULTIPLIER;

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
    if (defender->is_player())
    {
        // If the player does not yet have an automatic mob target, set it now.
        if (!core()->world()->player()->mob_target()) core()->world()->player()->set_mob_target(attacker->id());
    }
    else defender->add_hostility(attacker->id());   // Only do this on NON-player defenders, because obviously the player doesn't use the hostility vector.

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
        case WieldType::DUAL_WIELD: hit_multiplier = HIT_CHANCE_MULTIPLIER_DUAL_WIELD; break;
        case WieldType::SINGLE_WIELD: hit_multiplier = HIT_CHANCE_MULTIPLIER_SINGLE_WIELD; break;
        case WieldType::ONE_HAND_PLUS_SHIELD: case WieldType::UNARMED_PLUS_SHIELD: case WieldType::ONE_HAND_PLUS_EXTRA:
            hit_multiplier = HIT_CHANCE_MULTIPLIER_SWORD_AND_BOARD; break;
        default: break;
    }
    float to_hit = BASE_HIT_CHANCE_MELEE * hit_multiplier;

    // Check if the defender can attempt to block or parry.
    bool can_block = (wield_type_defender == WieldType::ONE_HAND_PLUS_SHIELD || wield_type_defender == WieldType::SHIELD_ONLY ||
        wield_type_defender == WieldType::UNARMED_PLUS_SHIELD) && !defender->tag(MobileTag::CannotBlock);
    bool can_parry = wield_type_defender != WieldType::UNARMED && wield_type_defender != WieldType::SHIELD_ONLY && wield_type_defender != WieldType::UNARMED_PLUS_SHIELD &&
        defender_melee && !defender->tag(MobileTag::CannotParry);

    // Check for Agile or Clumsy defender.
    if (defender->tag(MobileTag::Agile)) to_hit *= DEFENDER_TO_HIT_MODIFIER_AGILE;
    else if (defender->tag(MobileTag::Clumsy)) to_hit *= DEFENDER_TO_HIT_MODIFIER_CLUMSY;

    if (defender->tag(MobileTag::CannotDodge)) to_hit = 100;
    else to_hit *= defender->dodge_mod();

    bool evaded = false, blocked = false, parried = false;
    if (core()->rng()->frnd(100) <= to_hit)  // Evasion failed; the target was hit.
    {
        // Now to check if the defender can successfully parry this attack.
        if (can_parry)
        {
            float parry_chance = BASE_PARRY_CHANCE * defender->parry_mod();
            if (defender->tag(MobileTag::Agile) || attacker->tag(MobileTag::Clumsy)) parry_chance *= DEFENDER_PARRY_MODIFIER_AGILE;
            else if (defender->tag(MobileTag::Clumsy) || attacker->tag(MobileTag::Agile)) parry_chance *= DEFENDER_PARRY_MODIFIER_CLUMSY;
            if (core()->rng()->frnd(100) <= parry_chance) parried = true;
        }

        // If evasion and parry both fail, we can try to block. Parrying is better than blocking (parrying negates damage entirely) so there's no reason to run a block check
        // after a successful parry.
        if (!parried && can_block)
        {
            const float block_chance = BASE_BLOCK_CHANCE_MELEE * defender->block_mod();
            if (core()->rng()->frnd(100) <= block_chance) blocked = true;
        }
    }
    else evaded = true;

    // Determine where the defender was hit.
    EquipSlot def_location_hit_es;
    std::string def_location_hit_str;
    pick_hit_location(defender, &def_location_hit_es, &def_location_hit_str);

    if (parried || evaded)
    {
        if (parried)
        {
            if (player_can_see_attacker || player_can_see_defender)
            {
                if (defender_is_player) core()->message("{G}You parry the " + attacker_your_string + " " + weapon_name + "!");
                else core()->message((attacker_is_player ? "{Y}" : "{U}") + attacker_your_string_c + " " + weapon_name + " is parried by " + defender_name + ".");
            }
        }
        else
        {
            if (player_can_see_attacker || player_can_see_defender)
                core()->message((attacker_is_player ? "{Y}" : (defender_is_player ? "{U}" : "{U}")) + attacker_your_string_c + " " + weapon_name + " misses " + defender_name + ".");
        }
    }
    else
    {
        float damage = weapon_ptr->power() * BASE_MELEE_DAMAGE_MULTIPLIER;
        if (wield_type_attacker == WieldType::HAND_AND_A_HALF_2H) damage *= WEAPON_DAMAGE_MODIFIER_HAAH_2H;

        bool critical_hit = false, bleed = false, poison = false;
        float crit_chance = weapon_ptr->crit();
        if (wield_type_attacker == WieldType::SINGLE_WIELD) crit_chance *= CRIT_CHANCE_MULTIPLIER_SINGLE_WIELD;
        if (crit_chance >= 100.0f || core()->rng()->frnd(100) <= crit_chance)
        {
            critical_hit = true;
            bleed = true;
            damage *= 3;
        }

        if (attacker->tag(MobileTag::Anemic)) damage *= ATTACKER_DAMAGE_MODIFIER_ANEMIC;
        else if (attacker->tag(MobileTag::Feeble)) damage *= ATTACKER_DAMAGE_MODIFIER_FEEBLE;
        else if (attacker->tag(MobileTag::Puny)) damage *= ATTACKER_DAMAGE_MODIFIER_PUNY;
        else if (attacker->tag(MobileTag::Strong)) damage *= ATTACKER_DAMAGE_MODIFIER_STRONG;
        else if (attacker->tag(MobileTag::Brawny)) damage *= ATTACKER_DAMAGE_MODIFIER_BRAWNY;
        else if (attacker->tag(MobileTag::Vigorous)) damage *= ATTACKER_DAMAGE_MODIFIER_VIGOROUS;
        else if (attacker->tag(MobileTag::Mighty)) damage *= ATTACKER_DAMAGE_MODIFIER_MIGHTY;

        float damage_blocked = 0;
        if (def_location_hit_es == EquipSlot::BODY)
        {
            const std::shared_ptr<Item> body_armour = defender->equ()->get(EquipSlot::BODY);
            const std::shared_ptr<Item> outer_armour = defender->equ()->get(EquipSlot::ARMOUR);
            const EquipSlot outer_layer = (outer_armour ? EquipSlot::ARMOUR : EquipSlot::BODY);
            if (body_armour && outer_armour) damage_blocked = damage * body_armour->armour(outer_armour->power());
            else if (body_armour) damage_blocked = damage * body_armour->armour();
            else if (outer_armour) damage_blocked = damage * outer_armour->armour();
            damage_blocked = apply_damage_modifiers(damage_blocked, weapon_ptr, defender, outer_layer);
        }
        else
        {
            const std::shared_ptr<Item> armour_piece_hit = defender->equ()->get(def_location_hit_es);
            if (armour_piece_hit) damage_blocked = damage * armour_piece_hit->armour();
            damage_blocked = apply_damage_modifiers(damage_blocked, weapon_ptr, defender, def_location_hit_es);
        }
        if (blocked)
        {
            const std::shared_ptr<Item> shield_item = defender->equ()->get(EquipSlot::HAND_OFF);
            if (shield_item) damage_blocked += damage * shield_item->armour();
        }

        if (damage > 1) damage = MathX::mixup(std::round(damage), BASE_DAMAGE_VARIANCE);
        else if (damage > 0) damage = 1;
        if (damage_blocked > 1) damage_blocked = MathX::mixup(std::round(damage_blocked), BASE_ABSORPTION_VARIANCE);
        else if (damage_blocked > 0) damage_blocked = 1;
        if (damage_blocked >= damage) damage_blocked = damage;
        damage -= damage_blocked;

        const bool fatal = (damage >= defender->hp());
        if (player_is_here && (player_can_see_attacker || player_can_see_defender))
        {
            std::string damage_word = damage_str(damage, defender, false);
            std::string threshold_string = threshold_str(defender, damage, (attacker_is_player ? "{G}" : (defender_is_player ? "{R}" : "{U}")), (defender_is_player ? "{Y}" :
                (attacker_is_player ? "{y}" : "{U}")));
            std::string damage_colour = (attacker_is_player ? (damage > 0.0f ? "{G}" : "{y}") : (defender_is_player ? (damage > 0.0f ? "{R}" : "{Y}") : "{U}"));
            std::string absorb_str, block_str, death_str;
            if (damage_blocked)
            {
                std::shared_ptr<Item> armour_piece_hit = defender->equ()->get(blocked ? EquipSlot::HAND_OFF : def_location_hit_es);
                if (def_location_hit_es == EquipSlot::BODY && !blocked && defender->equ()->get(EquipSlot::ARMOUR))
                    armour_piece_hit = defender->equ()->get(EquipSlot::ARMOUR);
                std::string lessens_str, lessens_plural_str, lessening_str;
                if (damage < 1.0f)
                {
                    lessens_str = "absorbs";
                    lessens_plural_str = "absorb";
                    lessening_str = "absorbing";
                }
                else switch (core()->rng()->rnd(10))
                {
                    case 1: lessens_str = "mitigates"; lessens_plural_str = "mitigate"; lessening_str = "mitigating"; break;
                    case 2: lessens_str = "diminishes"; lessens_plural_str = "diminish"; lessening_str = "diminishing"; break;
                    case 3: lessens_str = "alleviates"; lessens_plural_str = "alleviate"; lessening_str = "alleviating"; break;
                    case 4: lessens_str = "deadens"; lessens_plural_str = "deaden"; lessening_str = "deadening"; break;
                    case 5: lessens_str = "dampens"; lessens_plural_str = "dampen"; lessening_str = "dampening"; break;
                    case 6: lessens_str = "dulls"; lessens_plural_str = "dull"; lessening_str = "dulling"; break;
                    case 7: lessens_str = "lessens"; lessens_plural_str = "lessen"; lessening_str = "lessening"; break;
                    case 8: lessens_str = "withstands"; lessens_plural_str = "withstand"; lessening_str = "withstanding"; break;
                    case 9: lessens_str = "endures"; lessens_plural_str = "endure"; lessening_str = "enduring"; break;
                    case 10: lessens_str = "takes"; lessens_plural_str = "take"; lessening_str = "taking"; break;
                }
                if (armour_piece_hit->tag(ItemTag::PluralName)) lessens_str = lessens_plural_str;
                if (blocked)
                {
                    const std::string blocks_str = (defender_is_player ? "block" : "blocks");
                    block_str = "{U}" + defender_name_c + " " + blocks_str + " with " + defender_your_string + " " + armour_piece_hit->name() + ", " + lessening_str + " the blow. ";
                }
                else absorb_str = " {U}" + defender_your_string_c + " " + armour_piece_hit->name() + " " + lessens_str + " the blow.";
            }

            if (fatal)
            {
                if (defender_is_player) death_str = " {M}You are slain!";
                else death_str = " {U}" + defender_name_c + (defender->tag(MobileTag::Unliving) ? " is destroyed!" : " is slain!");
            }
            core()->message(block_str + damage_colour + attacker_your_string_c + " " + weapon_name + " " + damage_word + " " + damage_colour +
                (blocked ? defender_name : defender_name_s + " " + def_location_hit_str) + "!" + threshold_string + absorb_str + " " +
                damage_number_str(damage, damage_blocked, critical_hit, bleed, poison) + death_str);
        }
        if (bleed) weapon_bleed_effect(defender, damage);
        defender->reduce_hp(damage);
    }
}
