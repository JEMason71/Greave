// actions/abilities.cpp -- Special abilities which can be used in combat.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/abilities.hpp"
#include "actions/combat.hpp"
#include "core/core.hpp"
#include "core/parser.hpp"
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


// Check cooldowns and availability of abilities.
void Abilities::abilities()
{
    const auto player = core()->world()->player();

    core()->message("{M}Available combat abilites:");

    auto display_ability = [&player](const std::string &name, bool stance_a, bool stance_b, bool stance_d, Buff::Type cd_buff, int cost_hp, int cost_sp, int cost_mp, bool needs_melee)
    {
        const CombatStance stance = player->stance();
        bool using_melee = Combat::using_melee(player);

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

        std::string ability_str = (can_use ? "{G}" : "{r}") + name + " ";
        
        if (!stance_a || !stance_b || !stance_d)
        {
            ability_str += "{W}<" + std::string(bad_stance ? "{r}" : "{G}") + "stance:";
            if (stance_a) ability_str += "A/";
            if (stance_b) ability_str += "B/";
            if (stance_d) ability_str += "D/";
            ability_str.pop_back();
            ability_str += "{W}> ";
        }

        if (needs_melee)
        {
            if (!stance_a || !stance_b || !stance_d) ability_str = ability_str.substr(0, ability_str.size() - 5) + "{W}, ";
            else ability_str += "{W}<";
            if (bad_gear) ability_str += "{r}"; else ability_str += "{G}";
            ability_str += "melee{W}> ";
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

    display_ability("Careful Aim", false, true, true, Buff::Type::CD_CAREFUL_AIM, 0, 0, CAREFUL_AIM_MP_COST, false);
    display_ability("Eye for an Eye", true, false, false, Buff::Type::CD_EYE_FOR_AN_EYE, EYE_FOR_AN_EYE_HP_COST, 0, 0, true);
}

// Attempt to use the Careful Aim ability.
void Abilities::careful_aim(bool confirm)
{
    const auto player = core()->world()->player();
    if (player->has_buff(Buff::Type::CD_CAREFUL_AIM))
    {
        core()->message("{m}You must wait a while before using the {M}Careful Aim {m}ability again.");
        return;
    }
    if (player->stance() == CombatStance::AGGRESSIVE)
    {
        core()->message("{m}Careful Aim can only be used in {M}balanced {m}or {M}defensive {m}stances.");
        return;
    }
    if (player->mp() < CAREFUL_AIM_MP_COST)
    {
        core()->message("{m}You do not have enough mana to use {M}Careful Aim{m}.");
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
        core()->message("{m}You must wait a while before using the {M}Eye for an Eye {m}ability again.");
        return;
    }
    if (player->stance() != CombatStance::AGGRESSIVE)
    {
        core()->message("{m}Eye for an Eye can only be used in an {M}aggressive {m}combat stance.");
        return;
    }
    if (!Combat::using_melee(player))
    {
        core()->message("{m}Eye for an Eye can only be used with {M}melee weapons{m}!");
        return;
    }
    if (player->hp() <= EYE_FOR_AN_EYE_HP_COST && !confirm)
    {
        core()->message("{m}You do not have enough hit points to use {M}Eye for an Eye{m}. You can force it, but that would result in your death!");
        core()->parser()->confirm_message();
        return;
    }

    core()->message("{M}Your vision goes red and you prepare for a brutal retaliatory strike.");
    player->set_buff(Buff::Type::CD_EYE_FOR_AN_EYE, EYE_FOR_AN_EYE_COOLDOWN);
    player->set_buff(Buff::Type::EYE_FOR_AN_EYE, EYE_FOR_AN_EYE_LENGTH, EYE_FOR_AN_EYE_MULTI, false, false);
    player->reduce_hp(EYE_FOR_AN_EYE_HP_COST);
}
