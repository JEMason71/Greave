// actions/eat-drink.cpp -- Eating food and drinking beverages.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/eat-drink.hpp"
#include "core/core.hpp"
#include "core/parser.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/player.hpp"
#include "world/time-weather.hpp"
#include "world/world.hpp"


// Drinks a specified inventory item.
void ActionEatDrink::drink(size_t inv_pos, bool confirm)
{
    const auto player = core()->world()->player();
    const auto item = player->inv()->get(inv_pos);
    const int liquid_available = item->charge();
    if (item->type() != ItemType::DRINK)
    {
        core()->message("{u}That isn't something you can drink!");
        return;
    }
    if (!liquid_available)
    {
        core()->message("{u}Your " + item->name(Item::NAME_FLAG_NO_COUNT) + " {u}is empty!");
        return;
    }

    const int thirst = player->thirst();
    const int water_space_left_in_body = std::max(1, 20 - thirst);
    const int liquid_consumed = std::min(water_space_left_in_body, liquid_available);
    if (thirst >= 20 && !confirm)
    {
        core()->message("{u}But you're not at all thirsty! Are you sure you want to drink anyway?");
        core()->parser()->confirm_message();
        return;
    }

    if (!player->pass_time(item->speed()))
    {
        core()->message("{r}You are interrupted while trying to drink!");
        return;
    }

    std::string water_str;
    if (liquid_consumed == liquid_available) water_str = "{U}You drink the last of the ";
    else water_str = "{U}You drink some ";
    water_str += "{C}" + item->liquid_type() + " {U}from your " + item->name(Item::NAME_FLAG_NO_COUNT) + "{U}";
    if (thirst + liquid_consumed <= 14) water_str += ", but you're still thirsty...";
    else if (thirst <= 14 && thirst + liquid_consumed > 14) water_str += ", quenching your thirst.";
    else water_str += ".";
    core()->message(water_str);
    player->add_water(liquid_consumed);
    item->set_charge(liquid_available - liquid_consumed);
    if (liquid_consumed == liquid_available) item->set_liquid("");
}

// Eats a specified inventory item.
void ActionEatDrink::eat(size_t inv_pos, bool confirm)
{
    const auto player = core()->world()->player();
    const auto item = player->inv()->get(inv_pos);
    if (item->type() != ItemType::FOOD)
    {
        core()->message("{y}That isn't edible!");
        return;
    }

    const int old_hunger = player->hunger();
    const int new_hunger = old_hunger + item->power();
    if (new_hunger > 25 && !confirm)
    {
        core()->message("{y}You're not really hungry enough to fit all that in. Are you sure you want to force it?");
        core()->parser()->confirm_message();
        return;
    }

    if (!player->pass_time(item->speed()))
    {
        core()->message("{r}You are interrupted while trying to eat!");
        return;
    }

    const bool last_item = (!item->tag(ItemTag::Stackable) || item->stack() == 1);
    std::string eat_str;
    if (old_hunger <= 4) eat_str = "{U}You wolf down ";
    else if (old_hunger <= 8) eat_str = "{U}You devour ";
    else if (new_hunger > 20) eat_str = "{U}You force yourself to eat ";
    else eat_str = "{U}You eat ";
    if (last_item) eat_str += "the last of your " + item->name(Item::NAME_FLAG_PLURAL | Item::NAME_FLAG_NO_COUNT);
    else eat_str  += item->name(Item::NAME_FLAG_THE | Item::NAME_FLAG_NO_COUNT);
    if (old_hunger <= 12 && new_hunger > 14) eat_str += "{U}. {G}That hit the spot!";
    else if (new_hunger <= 12) eat_str += "{U}, but you're {c}still hungry{U}...";
    else if (new_hunger > 25) eat_str += "{U}, feeling {c}extremely full and bloated{U}.";
    else if (new_hunger > 20) eat_str += "{U}, feeling {c}extremely full{U}.";
    else eat_str += "{U}, feeling satiated.";
    core()->message(eat_str);

    player->add_food(item->power());
    if (last_item) player->inv()->remove_item(inv_pos);
    else item->set_stack(item->stack() - 1);
}
