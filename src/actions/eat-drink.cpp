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
