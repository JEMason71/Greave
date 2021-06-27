// core/mathx.cpp -- Various utility functions that deal with math and number-related things.
// Copyright (c) 2009-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "core/mathx.hpp"


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
double MathX::round_to(double num, unsigned int digits)
{
    const double power = std::pow(10, digits);
    num *= power;
    const double rounded = std::round(num);
    return rounded / power;
}

// Rounds a float to two decimal places.
float MathX::round_to_two(float num) { return ::floorf(num * 100 + 0.5) / 100; }
