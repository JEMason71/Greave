// combat/combat.hpp -- Generic combat routines that apply to multiple types of combat.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Mobile;   // defined in world/mobile.hpp


class Combat
{
public:
    static std::string damage_str(unsigned int damage, std::shared_ptr<Mobile> def, bool heat); // Returns an appropriate damage string.
};
