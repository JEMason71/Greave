// core/random.cc -- The random number generator. Multiple instances of this class can be spawned if needed.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "core/random.h"

#include <random>

#pragma GCC diagnostic ignored "-Wunused-variable"
#include "3rdparty/pcg/randutils.hpp"
#pragma GCC diagnostic pop


// Constructor, sets up the PRNG.
Random::Random() { set_prand_seed(); }

// Returns a random number between min_float and max_float.
float Random::frnd(float min_float, float max_float)
{
    std::uniform_real_distribution<float> generator(min_float, max_float);
    return generator(m_pcg_rng);
}

// As above, but implicitly uses 1 as the minimum value.
float Random::frnd(float max_float) { return frnd(1, max_float); }

// Returns true if a random number between 1 and 100 is lower than or equal to the specified value.
bool Random::percent_check(unsigned int percent) { return rnd(1, 100) <= percent; }

// Returns a random number between min_int and max_int.
uint32_t Random::rnd(uint32_t min_int, uint32_t max_int)
{
    std::uniform_int_distribution<uint32_t> generator(min_int, max_int);
    return generator(m_pcg_rng);
}

// As above, but implicitly uses 1 as the minimum value.
uint32_t Random::rnd(uint32_t max_int) { return rnd(1, max_int); }

// 'Rolls' a number of dice with an optional modifier (e.g. 4d6+3).
int Random::roll(int num_dice, int die_faces, int modifier)
{
    if (!num_dice || !die_faces) return 0;
    int total = 0;
    for (int r = 0; r < num_dice; r++)
        total += rnd(1, die_faces);
    return total + modifier;
}

// Sets the PRNG seed for PCG.
void Random::set_prand_seed(uint32_t new_seed)
{
    if (new_seed) m_pcg_rng.seed(new_seed);
    else m_pcg_rng.seed(randutils::auto_seed_256{});
}
