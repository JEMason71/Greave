// actions/travel.cpp -- Actions allowing the player and NPCs to move around the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/doors.hpp"
#include "actions/look.hpp"
#include "actions/travel.hpp"
#include "combat/combat.hpp"
#include "core/core.hpp"
#include "core/mathx.hpp"
#include "core/parser.hpp"
#include "core/random.hpp"
#include "core/strx.hpp"
#include "world/player.hpp"
#include "world/room.hpp"
#include "world/world.hpp"


const int ActionTravel::FALL_1_STOREY_BLEED =           5;      // Intensity for the bleed room scar from a one-storey fall.
const int ActionTravel::FALL_1_STOREY_MIN_PERC =        20;     // Minimum % damage taken from falling one storey.
const int ActionTravel::FALL_1_STOREY_RNG_PERC =        50;     // Extra RNG % damage from one-storey fall.
const int ActionTravel::FALL_2_STOREY_BLEED =           10;
const int ActionTravel::FALL_2_STOREY_MIN_PERC =        50;
const int ActionTravel::FALL_2_STOREY_RNG_PERC =        70;
const int ActionTravel::FALL_3_STOREY_BLEED =           20;
const int ActionTravel::FALL_3_STOREY_MIN_PERC =        70;
const int ActionTravel::FALL_3_STOREY_RNG_PERC =        100;
const int ActionTravel::FALL_4_STOREY_BLEED =           25;
const int ActionTravel::FALL_4_STOREY_MIN_PERC =        90;
const int ActionTravel::FALL_4_STOREY_RNG_PERC =        00;
const int ActionTravel::FALL_5_STOREY_BLEED =           30;
const int ActionTravel::FALL_5_STOREY_MIN_PERC =        100;
const int ActionTravel::FALL_5_STOREY_RNG_PERC =        500;
const int ActionTravel::FALL_BLEED_DIVISOR_MAX =        20;     // The maximum amount of HP damage division from falling applied to each bleed tick.
const int ActionTravel::FALL_BLEED_DIVISOR_MIN =        10;     // The minimum amount of HP damage division from falling applied to each bleed tick.
const int ActionTravel::FALL_BLEED_INTENSITY_RANGE =    3;      // The variance range of the length of bleeds from falling.
const float ActionTravel::TRAVEL_TIME_DOUBLE =          120.0f; // The time (in seconds) it takes to travel across a double-length room link.
const float ActionTravel::TRAVEL_TIME_NORMAL =          30.0f;  // The time (in seconds) it takes to travel across a normal room link.
const float ActionTravel::TRAVEL_TIME_TRIPLE =          480.0f; // The time (in seconds) it takes to travel across a triple-length room link.


// Attempts to move from one Room to another.
bool ActionTravel::travel(std::shared_ptr<Mobile> mob, Direction dir, bool confirm)
{
    const uint32_t mob_loc = mob->location();
    const uint32_t player_loc = core()->world()->player()->location();
    const std::shared_ptr<Room> room = core()->world()->get_room(mob_loc);
    const bool is_player = mob->is_player();
    const uint32_t room_link = room->link(dir);

    if (!room_link)
    {
        if (is_player) core()->message("{y}You cannot travel {Y}" + StrX::dir_to_name(dir, StrX::DirNameType::TO_THE_ALT) + "{y}.");
        return false;
    }
    else if (room_link == Room::UNFINISHED)
    {
        if (is_player) core()->message("{y}That part of the game is {Y}currently unfinished{y}. Please come back later.");
        return false;
    }
    else if (room_link == Room::BLOCKED)
    {
        if (is_player) core()->message("{y}You are {Y}unable to proceeed {y}any further in that direction.");
        return false;
    }

    if (room->link_tag(dir, LinkTag::Openable) && !room->link_tag(dir, LinkTag::Open))
    {
        if (is_player) core()->message("{0}{m}(first opening the " + room->door_name(dir) + ")");
        const bool opened = ActionDoors::open_or_close(mob, dir, true);
        if (!opened) return false;
    }

    const bool sky = room->link_tag(dir, LinkTag::Sky);
    const bool sky2 = room->link_tag(dir, LinkTag::Sky2);
    const bool sky3 = room->link_tag(dir, LinkTag::Sky3);
    if ((sky || sky2 || sky3) && !confirm)
    {
        core()->message("{r}You risk taking damage or even dying from making a jump like that!");
        core()->parser()->confirm_message();
        return false;
    }

    float travel_time = TRAVEL_TIME_NORMAL;
    if (room->link_tag(dir, LinkTag::DoubleLength)) travel_time = TRAVEL_TIME_DOUBLE;
    else if (room->link_tag(dir, LinkTag::TripleLength)) travel_time = TRAVEL_TIME_TRIPLE;
    const bool success = mob->pass_time(travel_time);
    if (!success)
    {
        core()->message("{R}You are interrupted before you are able to leave!");
        return false;
    }

    const bool player_can_see = room->light(core()->world()->player());
    const std::string mob_name_the = (player_can_see ? mob->name(Mobile::NAME_FLAG_THE | Mobile::NAME_FLAG_CAPITALIZE_FIRST) : "Something");
    const std::string mob_name_a = (player_can_see ? mob->name(Mobile::NAME_FLAG_A | Mobile::NAME_FLAG_CAPITALIZE_FIRST) : "Something");

    if (sky || sky2 || sky3)
    {
        if (is_player) core()->message("{C}You take a {U}leap of faith{C}!");
        else if (mob_loc == player_loc) core()->message("{U}" + mob_name_the + " {U}takes a leap of faith " + StrX::dir_to_name(dir, StrX::DirNameType::TO_THE_ALT) + "!");
    }
    else if (!is_player && mob_loc == player_loc) core()->message("{U}" + mob_name_the + " {U}leaves " + StrX::dir_to_name(dir, StrX::DirNameType::TO_THE_ALT) + ".");

    mob->set_location(room_link);
    if (is_player) ActionLook::look(mob);
    else if (room_link == player_loc) core()->message("{U}" + mob_name_a + " {U}arrives " + StrX::dir_to_name(MathX::dir_invert(dir), StrX::DirNameType::FROM_THE_ALT) + ".");

    if (sky || sky2 || sky3)
    {
        int min_perc = 0, rng_perc = 0, blood_intensity = 0;
        if (sky3)
        {
            min_perc = FALL_3_STOREY_MIN_PERC;
            rng_perc = FALL_3_STOREY_RNG_PERC;
            blood_intensity = FALL_3_STOREY_BLEED;
        }
        else if (sky2)
        {
            min_perc = FALL_2_STOREY_MIN_PERC;
            rng_perc = FALL_2_STOREY_RNG_PERC;
            blood_intensity = FALL_2_STOREY_BLEED;
        }
        else
        {
            min_perc = FALL_1_STOREY_MIN_PERC;
            rng_perc = FALL_1_STOREY_RNG_PERC;
            blood_intensity = FALL_1_STOREY_BLEED;
        }
        float damage_perc = static_cast<float>(min_perc + core()->rng()->rnd(rng_perc)) / 100.0f;

        if (damage_perc > 0)
        {
            uint32_t hp_damage = std::round(static_cast<float>(mob->hp(true)) * damage_perc);
            mob->reduce_hp(hp_damage);
            if (is_player) core()->message("{R}You land badly, and the impact " + Combat::damage_str(hp_damage, mob, false) + " {R}you! {W}<{R}-" + StrX::intostr_pretty(hp_damage) +
                "{W}>");
            else
            {
                if (room_link == player_loc)
                {
                    if (player_can_see) core()->message("{U}" + mob_name_a + " {U}lands badly nearby with a painful crunch!");
                    else core()->message("{U}You hear the loud crunch of something landing badly nearby!");
                }
            }
            if (!mob->tag(MobileTag::ImmunityBleed))
            {
                core()->world()->get_room(mob->location())->add_scar(ScarType::BLOOD, MathX::mixup(blood_intensity, FALL_BLEED_INTENSITY_RANGE));
                mob->set_buff(Buff::Type::BLEED, blood_intensity, hp_damage / core()->rng()->rnd(FALL_BLEED_DIVISOR_MIN, FALL_BLEED_DIVISOR_MAX));
            }
            if (mob->hp() <= 0)
            {
                if (is_player) core()->message("{0}{M}Your bones are shattered from the impact, death is mercifully quick.");
                else if (player_can_see && room_link == player_loc) core()->message("{U}" + mob_name_the + " is slain instantly from the impact!");
            }
        }
        else if (is_player) core()->message("{g}Despite the distance fallen, you manage to land safely on your feet.");
    }

    return true;
}
