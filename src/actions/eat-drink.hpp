// actions/eat-drink.hpp -- Eating food and drinking beverages.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


class ActionEatDrink
{
public:
    static void drink(size_t inv_pos, bool confirm);    // Drinks a specified inventory item.
    static void eat(size_t inv_pos, bool confirm);      // Eats a specified inventory item.
    static void empty(size_t inv_pos, bool confirm);    // Empties a liquid container.
    static void fill(size_t inv_pos, bool confirm);     // Fills a liquid container.
    static void vomit(bool confirm);                    // Loses the contents of your stomach.

private:
    static const int    TIME_EMPTY_CONTAINER;           // The time taken to empty a water container.
    static const int    TIME_FILL_CONTAINER;            // The time taken to fill a water container.
    static const int    VOMIT_CHANCE_BLOAT_MAJOR;       // 1 in X chance of vomiting from severely over-eating.
    static const int    VOMIT_CHANCE_BLOAT_MINOR;       // 1 in X chance of vomiting from just over-eating a little.
    static const int    VOMIT_FOOD_LOSS_MAX;            // 1 to X food lost when vomiting.
    static const int    VOMIT_MINIMUM_FOOD_REMAINING;   // How much food to allow to remain after vomiting?
    static const int    VOMIT_MINIMUM_WATER_REMAINING;  // How much water to allow to remain after vomiting?
    static const int    VOMIT_SCAR_INTENSITY;           // Vomit type scar intensity for vomiting once.
    static const int    VOMIT_WATER_LOSS_MAX;           // 1 to X water lost when vomiting.
};
