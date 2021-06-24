// core/mathx.hpp -- Various utility functions that deal with math, random number generation and crypto.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


class MathX
{
public:
    static Direction    dir_invert(Direction dir);  // Inverts a Direction enum (north becomes south, etc.)
    static uint8_t      dir_invert(uint8_t dir);    // As above, but using integers.
};
