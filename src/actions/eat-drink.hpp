// actions/eat-drink.hpp -- Eating food and drinking beverages.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


class ActionEatDrink
{
public:
    static void drink(size_t inv_pos, bool confirm);    // Drinks a specified inventory item.
    static void eat(size_t inv_pos, bool confirm);      // Eats a specified inventory item.
};
