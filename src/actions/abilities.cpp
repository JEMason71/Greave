// actions/abilities.cpp -- Special abilities which can be used in combat.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/abilities.hpp"
#include "actions/combat.hpp"
#include "core/core.hpp"
#include "core/parser.hpp"
#include "core/random.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/player.hpp"
#include "world/world.hpp"


float   Abilities::CAREFUL_AIM_BONUS_HIT =      25; // The bonus hit% chance from using the Careful Aim ability.
int     Abilities::CAREFUL_AIM_COOLDOWN =       8;  // The length of the Careful Aim cooldown.
int     Abilities::CAREFUL_AIM_LENGTH =         2;  // How many buff ticks the Careful Aim ability lasts for.
int     Abilities::CAREFUL_AIM_MP_COST =        20; // The mana point cost for the Careful Aim ability.
float   Abilities::CAREFUL_AIM_TIME =           2;  // The time taken by the Careful Aim ability.
int     Abilities::EYE_FOR_AN_EYE_COOLDOWN =    30; // The cooldown for the Eye For An Eye ability.
int     Abilities::EYE_FOR_AN_EYE_HP_COST =     30; // The hit points cost for using Eye for an Eye.
int     Abilities::EYE_FOR_AN_EYE_LENGTH =      10; // The length of time the Eye For An Eye buff remains when activated but unused.
float   Abilities::EYE_FOR_AN_EYE_MULTI =       5;  // The damage multiplier for the Eye For An Eye ability.
int     Abilities::GRIT_COOLDOWN =              5;  // The cooldown for the Grit ability.
float   Abilities::GRIT_DAMAGE_REDUCTION =      30; // The % of damage reduced by using the Grit ability.
int     Abilities::GRIT_LENGTH =                30; // The Grit ability lasts this long, or until the player is hit by an attack.
int     Abilities::GRIT_SP_COST =               30; // The stamina point cost for the Grit ability.
float   Abilities::GRIT_TIME =                  2;  // The time taken by using the Grit ability.
int     Abilities::LADY_LUCK_COOLDOWN =         20; // The cooldown for the Lady Luck ability.
int     Abilities::LADY_LUCK_LENGTH =           60; // The buff/debuff time for the Lady Luck ability.
int     Abilities::LADY_LUCK_MP_COST =          50; // The mana cost for using the Lady Luck ability.
float   Abilities::LADY_LUCK_TIME =             2;  // The time taken by using the Lady Luck ability.


// Check cooldowns and availability of abilities.
void Abilities::abilities()
{
    const auto player = core()->world()->player();

    enum AbilityRequirements {
        NONE = 0,
        STANCE_A = 1,       // Requires aggressive stance.
        STANCE_B = 2,       // Requires balanced stance.
        STANCE_D = 4,       // Requires defensive stance.
        STANCE_ANY = 7,     // Any stance is fine.
        MELEE = 8,          // Requires melee weapon.
        ARMOUR_HEAVY = 16,  // Requires heavy outer armour.
        ARMOUR_MEDIUM = 32, // Requires medium outer armour.
        LUCKY_DICE = 64,    // Requires lucky dice.
    };

    auto display_ability = [&player](const std::string &name, Buff::Type cd_buff, int cost_hp, int cost_sp, int cost_mp, int flags, bool available)
    {
        const bool stance_a = (flags & STANCE_A);
        const bool stance_b = (flags & STANCE_B);
        const bool stance_d = (flags & STANCE_D);
        const bool needs_melee = (flags & MELEE);
        const bool needs_heavy = (flags & ARMOUR_HEAVY);
        const bool needs_medium = (flags & ARMOUR_MEDIUM);
        const bool needs_lucky_dice = (flags & LUCKY_DICE);

        const CombatStance stance = player->stance();
        bool using_melee = Combat::using_melee(player);
        const auto armour = player->equ()->get(EquipSlot::ARMOUR);
        const auto inner = player->equ()->get(EquipSlot::BODY);
        const bool using_heavy = (armour && armour->subtype() == ItemSub::HEAVY) || (inner && inner->subtype() == ItemSub::HEAVY);
        const bool using_medium = (armour && armour->subtype() == ItemSub::MEDIUM) || (inner && inner->subtype() == ItemSub::MEDIUM);
        const bool lucky_dice = true;   // todo: Add a check for lucky dice here, when they're added to the game.

        bool can_use = true, bad_stance = false, bad_cost = false, bad_buff = false, bad_gear = false;
        if ((!stance_a && stance == CombatStance::AGGRESSIVE) || (!stance_b && stance == CombatStance::BALANCED) || (!stance_d && stance == CombatStance::DEFENSIVE))
        {
            can_use = false;
            bad_stance = true;
        }
        if ((cost_hp && player->hp() < cost_hp) || (cost_mp && player->mp() < cost_mp) || (cost_sp && player->sp() < cost_sp))
        {
            can_use = false;
            bad_cost = true;
        }
        if (cd_buff != Buff::Type::NONE && player->has_buff(cd_buff))
        {
            can_use = false;
            bad_buff = true;
        }
        if (needs_melee && !using_melee)
        {
            can_use = false;
            bad_gear = true;
        }
        if (needs_medium || needs_heavy)
        {
            bool acceptable = false;

            if (needs_medium && using_medium) acceptable = true;
            if (needs_heavy && using_heavy) acceptable = true;

            if (!acceptable)
            {
                can_use = false;
                bad_gear = true;
            }
        }
        if (needs_lucky_dice && !lucky_dice)
        {
            can_use = false;
            bad_gear = true;
        }

        if (can_use != available) return;

        std::string ability_str = (can_use ? "{G}" : "{r}") + name + " ";
        bool angle_str = false;
        
        if ((stance_a || stance_b || stance_d) && !(stance_a && stance_b && stance_d))
        {
            ability_str += "{W}<" + std::string(bad_stance ? "{r}" : "{G}") + "stance:";
            if (stance_a) ability_str += "a/";
            if (stance_b) ability_str += "b/";
            if (stance_d) ability_str += "d/";
            ability_str.pop_back();
            ability_str += "{W}> ";
            angle_str = true;
        }

        if (needs_melee)
        {
            if (angle_str) ability_str = ability_str.substr(0, ability_str.size() - 5) + "{W}, ";
            else ability_str += "{W}<";
            if (bad_gear) ability_str += "{r}"; else ability_str += "{G}";
            ability_str += "melee{W}> ";
            angle_str = true;
        }
        
        if (needs_medium || needs_heavy)
        {
            if (angle_str) ability_str = ability_str.substr(0, ability_str.size() - 5) + "{W}, ";
            else ability_str += "{W}<";
            if (bad_gear) ability_str += "{r}"; else ability_str += "{G}";

            std::string armour_str;
            if (needs_medium) armour_str += "medium";
            if (needs_heavy)
            {
                if (armour_str.size()) armour_str += "/heavy";
                else armour_str += "heavy";
            }
            ability_str += armour_str + "{W}> ";
            angle_str = true;
        }

        if (needs_lucky_dice)
        {
            if (angle_str) ability_str = ability_str.substr(0, ability_str.size() - 5) + "{W}, ";
            else ability_str += "{W}<";
            if (bad_gear) ability_str += "{r}"; else ability_str += "{G}";
            ability_str += "dice{W}> ";
            angle_str = true;
        }

        if (cost_hp)
        {
            ability_str += "{W}[";
            if (bad_cost) ability_str += "{r}"; else ability_str += "{R}";
            ability_str += std::to_string(cost_hp) + "hp{W}] ";
        }

        if (cost_sp)
        {
            ability_str += "{W}[";
            if (bad_cost) ability_str += "{g}"; else ability_str += "{G}";
            ability_str += std::to_string(cost_sp) + "sp{W}] ";
        }

        if (cost_mp)
        {
            ability_str += "{W}[";
            if (bad_cost) ability_str += "{u}"; else ability_str += "{U}";
            ability_str += std::to_string(cost_mp) + "mp{W}] ";
        }

        if (bad_buff) ability_str += "{W}[{r}on cooldown{W}] ";

        ability_str.pop_back();
        core()->message("{0}" + ability_str);
    };

    for (int i = 0; i < 2; i++)
    {
        bool valid = (i == 0);
        if (valid) core()->message("{M}Available combat abilites:");
        else core()->message("{M}Unavailable abilities:");
        display_ability("CarefulAim", Buff::Type::CD_CAREFUL_AIM, 0, 0, CAREFUL_AIM_MP_COST, STANCE_B | STANCE_D, valid);
        display_ability("EyeForAnEye", Buff::Type::CD_EYE_FOR_AN_EYE, EYE_FOR_AN_EYE_HP_COST, 0, 0, STANCE_A | MELEE, valid);
        display_ability("Grit", Buff::Type::CD_GRIT, 0, GRIT_SP_COST, 0, STANCE_D | ARMOUR_HEAVY | ARMOUR_MEDIUM, valid);
        display_ability("LadyLuck", Buff::Type::CD_LADY_LUCK, 0, 0, LADY_LUCK_MP_COST, LUCKY_DICE | STANCE_ANY, valid);
    }
}

// Attempt to use the Careful Aim ability.
void Abilities::careful_aim(bool confirm)
{
    const auto player = core()->world()->player();
    if (player->has_buff(Buff::Type::CD_CAREFUL_AIM))
    {
        core()->message("{m}You must wait a while before using the {M}CarefulAim {m}ability again.");
        return;
    }
    if (player->stance() == CombatStance::AGGRESSIVE)
    {
        core()->message("{m}CarefulAim can only be used in {M}balanced {m}or {M}defensive {m}stances.");
        return;
    }
    if (player->mp() < CAREFUL_AIM_MP_COST)
    {
        core()->message("{m}You do not have enough mana to use {M}CarefulAim{m}.");
        return;
    }

    if (!player->pass_time(CAREFUL_AIM_TIME, !confirm)) return;
    if (player->is_dead()) return;

    core()->message("{M}You focus your mind, preparing for a precision strike.");
    player->set_buff(Buff::Type::CD_CAREFUL_AIM, CAREFUL_AIM_COOLDOWN);
    player->set_buff(Buff::Type::CAREFUL_AIM, CAREFUL_AIM_LENGTH, CAREFUL_AIM_BONUS_HIT, false, false);
    player->reduce_mp(CAREFUL_AIM_MP_COST);
}

// Attempt to use the Eye for an Eye ability.
void Abilities::eye_for_an_eye(bool confirm)
{
    const auto player = core()->world()->player();
    if (player->has_buff(Buff::Type::CD_EYE_FOR_AN_EYE))
    {
        core()->message("{m}You must wait a while before using the {M}EyeForAnEye {m}ability again.");
        return;
    }
    if (player->stance() != CombatStance::AGGRESSIVE)
    {
        core()->message("{m}EyeForAnEye can only be used in an {M}aggressive {m}combat stance.");
        return;
    }
    if (!Combat::using_melee(player))
    {
        core()->message("{m}EyeForAnEye can only be used with {M}melee weapons{m}!");
        return;
    }
    if (player->hp() <= EYE_FOR_AN_EYE_HP_COST && !confirm)
    {
        core()->message("{m}You do not have enough hit points to use {M}EyeForAnEye{m}. You can force it, but that would result in your death!");
        core()->parser()->confirm_message();
        return;
    }

    core()->message("{M}Your vision goes red and you prepare for a brutal retaliatory strike.");
    player->set_buff(Buff::Type::CD_EYE_FOR_AN_EYE, EYE_FOR_AN_EYE_COOLDOWN);
    player->set_buff(Buff::Type::EYE_FOR_AN_EYE, EYE_FOR_AN_EYE_LENGTH, EYE_FOR_AN_EYE_MULTI, false, false);
    player->reduce_hp(EYE_FOR_AN_EYE_HP_COST);
}

// Attempt to use the Grit ability.
void Abilities::grit(bool confirm)
{
    const auto player = core()->world()->player();
    const auto armour = player->equ()->get(EquipSlot::ARMOUR);
    const auto inner = player->equ()->get(EquipSlot::BODY);
    const bool using_heavy = (armour && armour->subtype() == ItemSub::HEAVY) || (inner && inner->subtype() == ItemSub::HEAVY);
    const bool using_medium = (armour && armour->subtype() == ItemSub::MEDIUM) || (inner && inner->subtype() == ItemSub::MEDIUM);
    
    if (player->has_buff(Buff::Type::CD_GRIT))
    {
        core()->message("{m}You must wait a while before using the {M}Grit {m}ability again.");
        return;
    }
    if (player->stance() != CombatStance::DEFENSIVE)
    {
        core()->message("{m}Grit can only be used in a {M}defensive {m}combat stance.");
        return;
    }
    if (!using_heavy && !using_medium)
    {
        core()->message("{m}Grit requires the use of {M}medium or heavy armour{m}.");
        return;
    }
    if (player->sp() < GRIT_SP_COST)
    {
        core()->message("{m}You do not have enough stamina points to use {M}Grit{m}.");
        return;
    }

    if (!player->pass_time(GRIT_TIME, !confirm)) return;
    if (player->is_dead()) return;
    core()->message("{M}You brace yourself for an incoming attack.");
    player->set_buff(Buff::Type::CD_GRIT, GRIT_COOLDOWN);
    player->set_buff(Buff::Type::GRIT, GRIT_TIME, GRIT_DAMAGE_REDUCTION, false, false);
    player->reduce_sp(GRIT_SP_COST);
}

// Attempt to use the Lady Luck ability.
void Abilities::lady_luck(size_t target, bool confirm)
{
    const auto mob = core()->world()->mob_vec(target);
    const auto player = core()->world()->player();
    const bool has_dice = true; // todo: check for magic dice, when they're added to the game

    if (player->has_buff(Buff::Type::CD_LADY_LUCK))
    {
        core()->message("{m}You must wait a while before using the {M}LadyLuck {m}ability again.");
        return;
    }
    if (!has_dice)
    {
        core()->message("{m}You don't have the correct {M}special dice {m}to use this ability.");
        return;
    }
    if (player->mp() < LADY_LUCK_MP_COST)
    {
        core()->message("{m}You do not have enough mana points to use {M}LadyLuck{m}.");
    }

    if (!player->pass_time(LADY_LUCK_TIME, !confirm)) return;
    if (player->is_dead()) return;

    core()->message("{M}You beseech Lady Luck for good fortune! {m}You roll the dice of fate...");
    player->set_buff(Buff::Type::CD_LADY_LUCK, LADY_LUCK_COOLDOWN);
    player->reduce_mp(LADY_LUCK_MP_COST);

    uint32_t dice[2] = { core()->rng()->rnd(6), core()->rng()->rnd(6) };
    const uint32_t total = dice[0] + dice[1];

    auto dice_string = [&dice](char colour_a, char colour_b) -> std::string {
            return "{0}{W}[{" + std::string(1, colour_a) + "}" + std::to_string(dice[0]) + "{W}][{" + std::string(1, colour_b) + "}" + std::to_string(dice[1]) + "{W}]";
    };

    if (dice[0] == 1 && dice[1] == 1)
    {
        core()->message(dice_string('R', 'R') + " " + StrX::rainbow_text("SNAKE EYES!", "gG") + " {R}You stumble...");
        player->set_tag(MobileTag::SnakeEyes);
        player->pass_time(LADY_LUCK_LENGTH, true);  // We'll allow this to be interrupted - the buff should last until the player takes damage. Yes, there'll be edge cases where this delay causes a poison or bleed tick, and thus they avoid taking critical damage from an enemy attack, but that's just... luck. :3c
        player->clear_tag(MobileTag::SnakeEyes);
        return;
    }
    if (dice[0] == 6 && dice[1] == 6)
    {
        core()->message(dice_string('G', 'G') + " " + StrX::rainbow_text("BOXCARS!", "RYGCUMRYG") + " {U}An opening presents itself...");
        player->set_tag(MobileTag::Boxcars);
        player->set_tag(MobileTag::FreeAttack);
        Combat::attack(player, mob);
        player->clear_tag(MobileTag::Boxcars);
        player->clear_tag(MobileTag::FreeAttack);   // This should be auto-cleared in Combat::attack(), but just in case...
        return;
    }
    if (total == 3 || total == 11)
    {
        core()->message(dice_string('U', 'U') + " {G}You feel a sudden moment of clarity...");
        player->set_buff(Buff::Type::CAREFUL_AIM, CAREFUL_AIM_LENGTH, CAREFUL_AIM_BONUS_HIT);
        return;
    }
    if (total == 4 || total == 10)
    {
        core()->message(dice_string('U', 'U') + " {G}You anticipate " + mob->name(Mobile::NAME_FLAG_POSSESSIVE | Mobile::NAME_FLAG_THE) + " {G}next move...");
        // todo: add quick roll buff here
        return;
    }
    if (dice[0] == dice[1])
    {
        core()->message(dice_string('C', 'C') + " {U}An opportunity presents itself for a rapid strike...");
        // todo: add rapid strike buff tag here
        player->set_tag(MobileTag::FreeAttack);
        Combat::attack(player, mob);
        player->clear_tag(MobileTag::FreeAttack);   // This should be auto-cleared in Combat::attack(), but just in case...
        return;
    }
    core()->message(dice_string('Y', 'Y') + " {Y}Nothing happens...");
}
