// core/mathx.cc -- Various utility functions that deal with math and number-related things.
// Copyright (c) 2009-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "core/core.h"
#include "core/mathx.h"

#include <cmath>


// Inverts a Direction enum (north becomes south, etc.)
Direction MathX::dir_invert(Direction dir)
{
    switch(dir)
    {
        case Direction::NORTH: return Direction::SOUTH;
        case Direction::SOUTH: return Direction::NORTH;
        case Direction::EAST: return Direction::WEST;
        case Direction::WEST: return Direction::EAST;
        case Direction::NORTHEAST: return Direction::SOUTHWEST;
        case Direction::NORTHWEST: return Direction::SOUTHEAST;
        case Direction::SOUTHEAST: return Direction::NORTHWEST;
        case Direction::SOUTHWEST: return Direction::NORTHEAST;
        case Direction::UP: return Direction::DOWN;
        case Direction::DOWN: return Direction::UP;
        default: return Direction::NONE;
    }
}

// As above, but using integers.
uint8_t MathX::dir_invert(uint8_t dir) { return static_cast<uint8_t>(dir_invert(static_cast<Direction>(dir))); }

// Rounds a float to a specified number of digits.
double MathX::round_to(double num, int digits)
{
    const double power = std::pow(10, digits);
    num *= power;
    const double rounded = std::round(num);
    return rounded / power;
}

// Fuzzes a number, giving an estimate (e.g. 123456 becoomes 100000).
uint32_t MathX::fuzz(uint32_t num)
{
    if (num >= 1000000000) { num = std::round(num / 100000000.0f); return num * 100000000; }
    if (num >= 100000000) { num = std::round(num / 10000000.0f); return num * 10000000; }
    if (num >= 10000000) { num = std::round(num / 1000000.0f); return num * 1000000; }
    if (num >= 1000000) { num = std::round(num / 100000.0f); return num * 100000; }
    if (num >= 100000) { num = std::round(num / 10000.0f); return num * 10000; }
    if (num >= 10000) { num = std::round(num / 1000.0f); return num * 1000; }
    if (num >= 1000) { num = std::round(num / 100.0f); return num * 100; }
    if (num >= 50) { num = std::round(num / 10.0f); return num * 10; }
    if (num >= 25) return num + static_cast<uint32_t>(abs((static_cast<int32_t>(num) % 5) - 5));
    return num;
}

// Mixes up an integer a little. High variance values (e.g. 10) mix up a little, low (e.g. 2) mix up a lot.
uint32_t MathX::mixup(uint32_t num, int variance)
{
    if (!num) return num;
    int64_t variance_value = num / variance;
    if (variance_value < 1) variance_value = 1;
    int64_t result = static_cast<int64_t>(num) + core()->rng()->rnd(variance_value * 2) - variance_value;
    if (result < 1) result = 1;
    return result;
}

// Rounds a float to two decimal places.
float MathX::round_to_two(float num) { return ::floorf(num * 100 + 0.5) / 100; }
