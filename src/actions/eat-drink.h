// actions/eat-drink.h -- Eating food and drinking beverages.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_ACTIONS_EAT_DRINK_H_
#define GREAVE_ACTIONS_EAT_DRINK_H_

#include <cstddef>


class ActionEatDrink
{
public:
    static void drink(size_t inv_pos, bool confirm);    // Drinks a specified inventory item.
    static void eat(size_t inv_pos, bool confirm);      // Eats a specified inventory item.
    static void empty(size_t inv_pos, bool confirm);    // Empties a liquid container.
    static void fill(size_t inv_pos, bool confirm);     // Fills a liquid container.
    static void vomit(bool confirm);                    // Loses the contents of your stomach.

private:
    static constexpr float  TIME_EMPTY_CONTAINER =          5;  // The time taken to empty a water container.
    static constexpr float  TIME_FILL_CONTAINER =           20; // The time taken to fill a water container.
    static constexpr int    VOMIT_CHANCE_BLOAT_MAJOR =      2;  // 1 in X chance of vomiting from severely over-eating.
    static constexpr int    VOMIT_CHANCE_BLOAT_MINOR =      8;  // 1 in X chance of vomiting from just over-eating a little.
    static constexpr int    VOMIT_FOOD_LOSS_MAX =           5;  // 1 to X food lost when vomiting.
    static constexpr int    VOMIT_MINIMUM_FOOD_REMAINING =  3;  // How much food to allow to remain after vomiting?
    static constexpr int    VOMIT_MINIMUM_WATER_REMAINING = 3;  // How much water to allow to remain after vomiting?
    static constexpr int    VOMIT_SCAR_INTENSITY =          5;  // Vomit type scar intensity for vomiting once.
    static constexpr int    VOMIT_WATER_LOSS_MAX =          2;  // 1 to X water lost when vomiting.
};

#endif  // GREAVE_ACTIONS_EAT_DRINK_H_
