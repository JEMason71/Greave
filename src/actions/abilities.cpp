// actions/abilities.cpp -- Special abilities which can be used in combat.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/abilities.hpp"
#include "core/core.hpp"
#include "world/player.hpp"
#include "world/world.hpp"


float   Abilities::CAREFUL_AIM_BONUS_HIT =  25; // The bonus hit% chance from using the Careful Aim ability.
int     Abilities::CAREFUL_AIM_COOLDOWN =   8;  // The length of the Careful Aim cooldown.
int     Abilities::CAREFUL_AIM_LENGTH =     2;  // How many buff ticks the Careful Aim ability lasts for.
float   Abilities::CAREFUL_AIM_TIME =       2;  // The time taken by the Careful Aim ability.
int     Abilities::MP_COST_CAREFUL_AIM =    20; // The mana point cost for the Careful Aim ability.


// Check cooldowns and availability of abilities.
void Abilities::abilities()
{
    const auto player = core()->world()->player();

    core()->message("{M}Available combat abilites:");

    auto display_ability = [&player](const std::string &name, bool stance_a, bool stance_b, bool stance_d, Buff::Type cd_buff, int cost_hp, int cost_sp, int cost_mp)
    {
        const CombatStance stance = player->stance();

        bool can_use = true, bad_stance = false, bad_cost = false, bad_buff = false;
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

        std::string ability_str = (can_use ? "{G}" : "{R}") + name + " ";
        
        if (!stance_a || !stance_b || !stance_d)
        {
            ability_str += "{W}<" + std::string(bad_stance ? "{R}" : "{G}") + "stance:";
            if (stance_a) ability_str += "A/";
            if (stance_b) ability_str += "B/";
            if (stance_d) ability_str += "D/";
            ability_str.pop_back();
            ability_str += "{W}> ";
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

        if (bad_buff) ability_str += "{r}[{R}on cooldown{r}] ";

        ability_str.pop_back();
        core()->message("{0}" + ability_str);
    };

    display_ability("Careful Aim", false, true, true, Buff::Type::CD_CAREFUL_AIM, 0, 0, MP_COST_CAREFUL_AIM);
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
    if (player->mp() < MP_COST_CAREFUL_AIM)
    {
        core()->message("{m}You do not have enough mana to use {M}Careful Aim{m}.");
        return;
    }

    if (!player->pass_time(CAREFUL_AIM_TIME, !confirm)) return;
    if (player->is_dead()) return;

    core()->message("{M}You focus your mind, preparing for a precision strike.");
    player->set_buff(Buff::Type::CD_CAREFUL_AIM, CAREFUL_AIM_COOLDOWN);
    player->set_buff(Buff::Type::CAREFUL_AIM, CAREFUL_AIM_LENGTH, CAREFUL_AIM_BONUS_HIT, false, false);
    player->reduce_mp(MP_COST_CAREFUL_AIM);
}
