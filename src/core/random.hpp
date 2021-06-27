// core/random.hpp -- The random number generator. Multiple instances of this class can be spawned if needed.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"
#include "3rdparty/pcg/pcg_random.hpp"


class Random
{
                    Random();                                   // Constructor, sets up the PRNG.
    float           frnd(float min_float, float max_float);     // Returns a random number between min_float and max_float.
    float           frnd(float max_float);                      // As above, but implicitly uses 1 as the minimum value.
    unsigned int    rnd(unsigned int min_int, unsigned int max_int);    // Returns a random number between min_int and max_int.
    unsigned int    rnd(unsigned int max_int);                  // As above, but implicitly uses 1 as the minimum value.
    int             roll(unsigned int num_dice, unsigned int die_faces, int modifier = 0);  // 'Rolls' a number of dice with an optional modifier (e.g. 4d6+3).
    void            set_prand_seed(unsigned int new_seed = 0);  // Sets the PRNG seed for PCG.

    pcg32   m_pcg_rng;  // The PCG pseudo-random number generator.
};
