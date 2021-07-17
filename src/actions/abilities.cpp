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


float   Abilities::CAREFUL_AIM_BONUS_HIT =          25; // The bonus hit% chance from using the Careful Aim ability.
int     Abilities::CAREFUL_AIM_COOLDOWN =           8;  // The length of the Careful Aim cooldown.
int     Abilities::CAREFUL_AIM_LENGTH =             2;  // How many buff ticks the Careful Aim ability lasts for.
int     Abilities::CAREFUL_AIM_MP_COST =            20; // The mana point cost for the Careful Aim ability.
float   Abilities::CAREFUL_AIM_TIME =               2;  // The time taken by the Careful Aim ability.
int     Abilities::EYE_FOR_AN_EYE_COOLDOWN =        30; // The cooldown for the Eye For An Eye ability.
int     Abilities::EYE_FOR_AN_EYE_HP_COST =         30; // The hit points cost for using Eye for an Eye.
int     Abilities::EYE_FOR_AN_EYE_LENGTH =          10; // The length of time the Eye For An Eye buff remains when activated but unused.
float   Abilities::EYE_FOR_AN_EYE_MULTI =           5;  // The damage multiplier for the Eye For An Eye ability.
int     Abilities::GRIT_COOLDOWN =                  5;  // The cooldown for the Grit ability.
float   Abilities::GRIT_DAMAGE_REDUCTION =          30; // The % of damage reduced by using the Grit ability.
int     Abilities::GRIT_LENGTH =                    30; // The Grit ability lasts this long, or until the player is hit by an attack.
int     Abilities::GRIT_SP_COST =                   30; // The stamina point cost for the Grit ability.
float   Abilities::GRIT_TIME =                      2;  // The time taken by using the Grit ability.
int     Abilities::HEADLONG_STRIKE_ATTACK_SPEED =   20; // The % of an attack's normal speed that it takes to do a Headlong Strike attack.
int     Abilities::HEADLONG_STRIKE_COOLDOWN =       6;  // The cooldown for the Headlong Strike ability.
int     Abilities::HEADLONG_STRIKE_HP_COST =        10; // The hit points cost to use the Headlong Strike abiliy.
int     Abilities::LADY_LUCK_COOLDOWN =             20; // The cooldown for the Lady Luck ability.
int     Abilities::LADY_LUCK_LENGTH =               60; // The buff/debuff time for the Lady Luck ability.
int     Abilities::LADY_LUCK_MP_COST =              50; // The mana cost for using the Lady Luck ability.
float   Abilities::LADY_LUCK_TIME =                 2;  // The time taken by using the Lady Luck ability.
int     Abilities::QUICK_ROLL_BONUS_DODGE =         40; // The bonus dodge% chance from using the Quick Roll ability.
int     Abilities::QUICK_ROLL_COOLDOWN =            8;  // The cooldown for the Quick Roll ability.
int     Abilities::QUICK_ROLL_LENGTH =              5;  // The length of time the Quick Roll buff remains when activated, but before an enemy attack is made.
int     Abilities::QUICK_ROLL_SP_COST =             25; // The stamina point cost for the Quick Roll ability.
int     Abilities::QUICK_ROLL_TIME =                4;  // The time it takes to do a Quick Roll.
float   Abilities::RAPID_STRIKE_ACCURACY_PENALTY =  20; // The % accuracy penalty for a Rapid Strike.
float   Abilities::RAPID_STRIKE_ATTACK_SPEED =      20; // The % of an attack's normal speed that it takes to do a Rapid Strike attack.
int     Abilities::RAPID_STRIKE_COOLDOWN =          6;  // The cooldown for the Rapid Strike ability.
int     Abilities::RAPID_STRIKE_SP_COST =           50; // The stamina points cost for the Rapid Strike ability.
int     Abilities::SHIELD_WALL_BLOCK_BONUS  =       70; // The % bonus to blocking an attack with Shield Wall.
int     Abilities::SHIELD_WALL_COOLDOWN =           6;  // The cooldwon for the Shield Wall ability.
int     Abilities::SHIELD_WALL_LENGTH =             20; // The length of time the Shield Wall buff remains while activated, but before an enemy attack is made.
int     Abilities::SHIELD_WALL_SP_COST =            20; // The stamina points cost for the Shield Wall ability.
int     Abilities::SHIELD_WALL_TIME =               2;  // The time taken to use the Shield Wall ability.
float   Abilities::SNAP_SHOT_ACCURACY_PENALTY =     20; // The % accuracy penalty for a Snap Shot.
float   Abilities::SNAP_SHOT_ATTACK_SPEED =         20; // The % of an attack's normal speed that it takes to do a Snap Shot attack.
int     Abilities::SNAP_SHOT_COOLDOWN =             6;  // The cooldown for the Snap Shot ability.
int     Abilities::SNAP_SHOT_SP_COST =              50; // The stamina points cost for the Snap Shot ability.


// Check cooldowns and availability of abilities.
void Abilities::abilities()
{
    const auto player = core()->world()->player();

    enum AbilityRequirements {
        NONE = 0,
        STANCE_A = 1,           // Requires aggressive stance.
        STANCE_B = 2,           // Requires balanced stance.
        STANCE_D = 4,           // Requires defensive stance.
        STANCE_ANY = 7,         // Any stance is fine.
        MELEE = 8,              // Requires melee weapon.
        RANGED = 16,            // Requires ranged weapon.
        SHIELD = 32,            // Requires a shield.
        ARMOUR_HEAVY = 64,      // Requires heavy armour.
        ARMOUR_MEDIUM = 128,    // Requires medium armour.
        ARMOUR_LIGHT = 256,     // Requires light armour.
        ARMOUR_NO_HEAVY = 512,  // Requires NOT using heavy armour.
        LUCKY_DICE = 1024,      // Requires lucky dice.
    };

    auto display_ability = [&player](const std::string &name, Buff::Type cd_buff, int cost_hp, int cost_sp, int cost_mp, int flags, bool available)
    {
        const bool stance_a = (flags & STANCE_A);
        const bool stance_b = (flags & STANCE_B);
        const bool stance_d = (flags & STANCE_D);
        const bool needs_melee = (flags & MELEE);
        const bool needs_ranged = (flags & RANGED);
        const bool needs_shield = (flags & SHIELD);
        const bool needs_heavy = (flags & ARMOUR_HEAVY);
        const bool needs_medium = (flags & ARMOUR_MEDIUM);
        const bool needs_light = (flags & ARMOUR_LIGHT);
        const bool no_heavy = (flags & ARMOUR_NO_HEAVY);
        const bool needs_lucky_dice = (flags & LUCKY_DICE);

        const CombatStance stance = player->stance();
        const bool using_melee = player->using_melee();
        const bool using_ranged = player->using_ranged();
        const bool using_heavy = player->wearing_armour(ItemSub::HEAVY);
        const bool using_medium = player->wearing_armour(ItemSub::MEDIUM);
        const bool using_light = player->wearing_armour(ItemSub::LIGHT) || player->wearing_armour(ItemSub::NONE);
        const bool using_shield = player->using_shield();
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
        if ((needs_melee && !using_melee) || (needs_ranged && !using_ranged) || (no_heavy && using_heavy) || (needs_lucky_dice && !lucky_dice) || (needs_shield && !using_shield))
        {
            can_use = false;
            bad_gear = true;
        }
        if (needs_light || needs_medium || needs_heavy)
        {
            bool acceptable = false;

            if (needs_light && using_light) acceptable = true;
            if (needs_medium && using_medium) acceptable = true;
            if (needs_heavy && using_heavy) acceptable = true;

            if (!acceptable)
            {
                can_use = false;
                bad_gear = true;
            }
        }

        if (can_use != available) return;

        std::string ability_str = (can_use ? "{C}" : "{c}") + name + " ";
        bool angle_str = false;

        if ((stance_a || stance_b || stance_d) && !(stance_a && stance_b && stance_d))
        {
            ability_str += "{W}<" + std::string(bad_stance ? "{c}" : "{C}") + "stance:";
            if (stance_a) ability_str += "a/";
            if (stance_b) ability_str += "b/";
            if (stance_d) ability_str += "d/";
            ability_str.pop_back();
            ability_str += "{W}> ";
            angle_str = true;
        }

        if (needs_melee || needs_ranged)
        {
            if (angle_str) ability_str = ability_str.substr(0, ability_str.size() - 5) + "{W}, ";
            else ability_str += "{W}<";
            if (bad_gear) ability_str += "{c}"; else ability_str += "{C}";
            ability_str += (needs_melee ? "melee{W}> " : "ranged{W}> ");
            angle_str = true;
        }

        if (needs_medium || needs_heavy)
        {
            if (angle_str) ability_str = ability_str.substr(0, ability_str.size() - 5) + "{W}, ";
            else ability_str += "{W}<";
            if (bad_gear) ability_str += "{c}"; else ability_str += "{C}";

            std::string armour_str;
            if (needs_light) armour_str += "light";
            if (needs_medium)
            {
                if (armour_str.size()) armour_str += "/medium";
                else armour_str += "medium";
            }
            if (needs_heavy)
            {
                if (armour_str.size()) armour_str += "/heavy";
                else armour_str += "heavy";
            }
            ability_str += armour_str + "{W}> ";
            angle_str = true;
        }

        if (needs_shield)
        {
            if (angle_str) ability_str = ability_str.substr(0, ability_str.size() - 5) + "{W}, ";
            else ability_str += "{W}<";
            if (bad_gear) ability_str += "{c}"; else ability_str += "{C}";
            ability_str += "shield{W}> ";
            angle_str = true;
        }

        if (needs_lucky_dice)
        {
            if (angle_str) ability_str = ability_str.substr(0, ability_str.size() - 5) + "{W}, ";
            else ability_str += "{W}<";
            if (bad_gear) ability_str += "{c}"; else ability_str += "{C}";
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

        if (bad_buff) ability_str += "{W}({c}on cooldown{W}) ";

        ability_str.pop_back();
        if (!available) StrX::find_and_replace(ability_str, "{W}", "{w}");
        core()->message("{0}" + ability_str);
    };

    for (int i = 0; i < 2; i++)
    {
        bool valid = (i == 0);
        if (valid) core()->message("{M}Available combat abilities:");
        else core()->message("{M}Unavailable abilities:");
        display_ability("CarefulAim", Buff::Type::CD_CAREFUL_AIM, 0, 0, CAREFUL_AIM_MP_COST, STANCE_B | STANCE_D, valid);
        display_ability("EyeForAnEye", Buff::Type::CD_EYE_FOR_AN_EYE, EYE_FOR_AN_EYE_HP_COST, 0, 0, STANCE_A | MELEE, valid);
        display_ability("Grit", Buff::Type::CD_GRIT, 0, GRIT_SP_COST, 0, STANCE_D | ARMOUR_HEAVY | ARMOUR_MEDIUM, valid);
        display_ability("HeadlongStrike", Buff::Type::CD_HEADLONG_STRIKE, HEADLONG_STRIKE_HP_COST, 0, 0, STANCE_A | MELEE, valid);
        display_ability("LadyLuck", Buff::Type::CD_LADY_LUCK, 0, 0, LADY_LUCK_MP_COST, LUCKY_DICE | STANCE_ANY, valid);
        display_ability("QuickRoll", Buff::Type::CD_QUICK_ROLL, 0, QUICK_ROLL_SP_COST, 0, STANCE_B | STANCE_D | ARMOUR_LIGHT | ARMOUR_MEDIUM | ARMOUR_NO_HEAVY, valid);
        display_ability("RapidStrike", Buff::Type::CD_RAPID_STRIKE, 0, RAPID_STRIKE_SP_COST, 0, STANCE_B | MELEE, valid);
        display_ability("ShieldWall", Buff::Type::CD_SHIELD_WALL, 0, SHIELD_WALL_SP_COST, 0, STANCE_D | SHIELD, valid);
        display_ability("SnapShot", Buff::Type::CD_SNAP_SHOT, 0, SNAP_SHOT_SP_COST, 0, STANCE_B | RANGED, valid);
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
    if (!player->using_melee())
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

    core()->message("{M}Your vision goes red and you prepare for a brutal retaliatory strike! " + Combat::damage_number_str(EYE_FOR_AN_EYE_HP_COST, 0, false, false, false));
    player->set_buff(Buff::Type::CD_EYE_FOR_AN_EYE, EYE_FOR_AN_EYE_COOLDOWN);
    player->set_buff(Buff::Type::EYE_FOR_AN_EYE, EYE_FOR_AN_EYE_LENGTH, EYE_FOR_AN_EYE_MULTI, false, false);
    player->reduce_hp(EYE_FOR_AN_EYE_HP_COST);
}

// Attempt to use the Grit ability.
void Abilities::grit(bool confirm)
{
    const auto player = core()->world()->player();

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
    if (!player->wearing_armour(ItemSub::HEAVY) && !player->wearing_armour(ItemSub::MEDIUM))
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
    player->set_buff(Buff::Type::GRIT, GRIT_LENGTH, GRIT_DAMAGE_REDUCTION, false, false);
    player->reduce_sp(GRIT_SP_COST);
}

// Attempt to use the HeadlongStrike ability.
void Abilities::headlong_strike(size_t target, bool confirm)
{
    const auto player = core()->world()->player();
    const auto mob = core()->world()->mob_vec(target);
    if (player->has_buff(Buff::Type::CD_HEADLONG_STRIKE))
    {
        core()->message("{m}You must wait a while before using the {M}HeadlongStrike {m}ability again.");
        return;
    }
    if (player->stance() != CombatStance::AGGRESSIVE)
    {
        core()->message("{m}HeadlongStrike can only be used in an {M}aggressive {m}combat stance.");
        return;
    }
    if (!player->using_melee())
    {
        core()->message("{m}HeadlongStrike can only be used with {M}melee weapons{m}!");
        return;
    }
    if (player->hp() <= HEADLONG_STRIKE_HP_COST && !confirm)
    {
        core()->message("{m}You do not have enough hit points to use {M}HeadlongStrike{m}. You can force it, but that would result in your death!");
        core()->parser()->confirm_message();
        return;
    }

    core()->message("{M}Disregarding your own safety, you lunge into an aggressive attack! " + Combat::damage_number_str(HEADLONG_STRIKE_HP_COST, 0, false, false, false));
    if (player->is_dead()) return;
    player->set_buff(Buff::Type::CD_HEADLONG_STRIKE, HEADLONG_STRIKE_COOLDOWN);
    player->set_tag(MobileTag::HeadlongStrike);
    Combat::attack(player, mob);
    player->clear_tag(MobileTag::HeadlongStrike);
    player->reduce_hp(HEADLONG_STRIKE_HP_COST);
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
        player->set_buff(Buff::Type::QUICK_ROLL, QUICK_ROLL_TIME, QUICK_ROLL_BONUS_DODGE, false, false);
        return;
    }
    if (dice[0] == dice[1])
    {
        const bool ranged = player->using_ranged();
        if (ranged)
        {
            core()->message(dice_string('C', 'C') + " {U}An opportunity presents itself for a snap shot...");
            player->set_tag(MobileTag::SnapShot);
        }
        else
        {
            core()->message(dice_string('C', 'C') + " {U}An opportunity presents itself for a rapid strike...");
            player->set_tag(MobileTag::RapidStrike);
        }
        player->set_tag(MobileTag::FreeAttack);
        Combat::attack(player, mob);
        player->clear_tag(MobileTag::SnapShot);
        player->clear_tag(MobileTag::RapidStrike);
        player->clear_tag(MobileTag::FreeAttack);   // This should be auto-cleared in Combat::attack(), but just in case...
        return;
    }
    core()->message(dice_string('Y', 'Y') + " {Y}Nothing happens...");
}

// Attempt to use the Quick Roll ability.
void Abilities::quick_roll(bool confirm)
{
    const auto player = core()->world()->player();
    if (player->has_buff(Buff::Type::CD_QUICK_ROLL))
    {
        core()->message("{m}You must wait a while before using the {M}QuickRoll {m}ability again.");
        return;
    }
    if (player->wearing_armour(ItemSub::HEAVY))
    {
        core()->message("{m}You armour is too heavy to be able to {M}QuickRoll{m}.");
        return;
    }
    if (player->sp() < QUICK_ROLL_SP_COST)
    {
        core()->message("{m}You do not have enough stamina points to use {M}QuickRoll{m}.");
        return;
    }
    if (player->stance() == CombatStance::AGGRESSIVE)
    {
        core()->message("{m}QuickRoll can only be used in {M}balanced {m}or {M}defensive {m}stances.");
        return;
    }

    if (!player->pass_time(QUICK_ROLL_TIME, !confirm)) return;
    if (player->is_dead()) return;

    core()->message("{U}You make a quick combat roll, attempting to dodge an incoming attack.");
    player->set_buff(Buff::Type::CD_QUICK_ROLL, QUICK_ROLL_COOLDOWN);
    player->set_buff(Buff::Type::QUICK_ROLL, QUICK_ROLL_LENGTH, QUICK_ROLL_BONUS_DODGE, false, false);
    player->reduce_sp(QUICK_ROLL_SP_COST);
}

// Attempt to use the Rapid Strike ability.
void Abilities::rapid_strike(size_t target)
{
    const auto player = core()->world()->player();
    if (player->has_buff(Buff::Type::CD_RAPID_STRIKE))
    {
        core()->message("{m}You must wait a while before using the {M}RapidStrike {m}ability again.");
        return;
    }
    if (player->stance() != CombatStance::BALANCED)
    {
        core()->message("{m}RapidStrike can only be used in a {M}balanced {m}combat stance.");
        return;
    }
    if (player->sp() < RAPID_STRIKE_SP_COST)
    {
        core()->message("{m}You do not have enough stamina points to use {M}RapidStrike{m}.");
        return;
    }
    if (!player->using_melee())
    {
        core()->message("{m}RapidStrike can only be used with {M}melee weapons{m}.");
        return;
    }

    const auto mob = core()->world()->mob_vec(target);
    core()->message("{M}You strike rapidly at " + mob->name(Mobile::NAME_FLAG_THE) + "{M}!");
    player->reduce_sp(RAPID_STRIKE_SP_COST);
    player->set_buff(Buff::Type::CD_RAPID_STRIKE, RAPID_STRIKE_COOLDOWN);
    player->set_tag(MobileTag::RapidStrike);
    Combat::attack(player, mob);
    player->clear_tag(MobileTag::RapidStrike);
}

// Attempt to use the Shield Wall ability.
void Abilities::shield_wall(bool confirm)
{
    const auto player = core()->world()->player();
    if (player->has_buff(Buff::Type::CD_SHIELD_WALL))
    {
        core()->message("{m}You must wait a while before using the {M}ShieldWall {m}ability again.");
        return;
    }
    if (player->stance() != CombatStance::DEFENSIVE)
    {
        core()->message("{m}ShieldWall can only be used in a {M}defensive {m}combat stance.");
        return;
    }
    if (player->sp() < SHIELD_WALL_SP_COST)
    {
        core()->message("{m}You do not have enough stamina points to use {M}ShieldWall{m}.");
        return;
    }
    if (!player->using_shield())
    {
        core()->message("{m}ShieldWall can only be used with a {M}shield{m}.");
        return;
    }

    if (!player->pass_time(SHIELD_WALL_TIME, !confirm)) return;
    if (player->is_dead()) return;
    core()->message("{M}You brace yourself behind your shield for an incoming blow.");
    player->set_buff(Buff::Type::CD_SHIELD_WALL, SHIELD_WALL_COOLDOWN);
    player->set_buff(Buff::Type::SHIELD_WALL, SHIELD_WALL_LENGTH, SHIELD_WALL_BLOCK_BONUS, false, false);
    player->reduce_sp(SHIELD_WALL_SP_COST);
}

// Attempt to use the Snap Shot ability.
void Abilities::snap_shot(size_t target)
{
    const auto player = core()->world()->player();
    if (player->has_buff(Buff::Type::CD_SNAP_SHOT))
    {
        core()->message("{m}You must wait a while before using the {M}SnapShot {m}ability again.");
        return;
    }
    if (player->stance() != CombatStance::BALANCED)
    {
        core()->message("{m}SnapShot can only be used in a {M}balanced {m}combat stance.");
        return;
    }
    if (player->sp() < SNAP_SHOT_SP_COST)
    {
        core()->message("{m}You do not have enough stamina points to use {M}SnapShot{m}.");
        return;
    }
    if (!player->using_ranged())
    {
        core()->message("{m}SnapShot can only be used with {M}ranged weapons{m}.");
        return;
    }

    const auto mob = core()->world()->mob_vec(target);
    core()->message("{M}You take a quick snap shot at " + mob->name(Mobile::NAME_FLAG_THE) + "{M}!");
    player->reduce_sp(SNAP_SHOT_SP_COST);
    player->set_buff(Buff::Type::CD_SNAP_SHOT, SNAP_SHOT_COOLDOWN);
    player->set_tag(MobileTag::SnapShot);
    Combat::attack(player, mob);
    player->clear_tag(MobileTag::SnapShot);
}
