// core/random.h -- The random number generator. Multiple instances of this class can be spawned if needed.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_CORE_RANDOM_H_
#define GREAVE_CORE_RANDOM_H_

#include "3rdparty/pcg/pcg_random.hpp"

#include <cstdint>


class Random
{
public:
                Random();                                   // Constructor, sets up the PRNG.
    float       frnd(float min_float, float max_float);     // Returns a random number between min_float and max_float.
    float       frnd(float max_float);                      // As above, but implicitly uses 1 as the minimum value.
    bool        percent_check(unsigned int percent);        // Returns true if a random number between 1 and 100 is lower than or equal to the specified value.
    uint32_t    rnd(uint32_t min_int, uint32_t max_int);    // Returns a random number between min_int and max_int.
    uint32_t    rnd(uint32_t max_int);                      // As above, but implicitly uses 1 as the minimum value.
    int         roll(int num_dice, int die_faces, int modifier = 0);    // 'Rolls' a number of dice with an optional modifier (e.g. 4d6+3).
    void        set_prand_seed(uint32_t new_seed = 0);      // Sets the PRNG seed for PCG.

    pcg32       pcg_rng_;   // The PCG pseudo-random number generator.
};

#endif  // GREAVE_CORE_RANDOM_H_
