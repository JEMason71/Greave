// core/mathx.hpp -- Various utility functions that deal with math and number-related things.
// Copyright (c) 2009-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


class MathX
{
public:
    static Direction    dir_invert(Direction dir);              // Inverts a Direction enum (north becomes south, etc.)
    static uint8_t      dir_invert(uint8_t dir);                // As above, but using integers.
    static uint32_t     fuzz(uint32_t num);                     // Fuzzes a number, giving an estimate (e.g. 123456 becoomes 100000).
    static double       round_to(double num, uint32_t digits);  // Rounds a float to a specified number of digits.
    static float        round_to_two(float num);                // Rounds a float to two decimal places.
};
