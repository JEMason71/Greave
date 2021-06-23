// world/npc.cpp -- The NPC class is derived from Mobile, and defines non-player characters, monsters and beasts in the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "world/npc.hpp"


// Returns Mobile::Type::NPC, so we can check what type of Mobile this is when using a Mobile pointer.
Mobile::Type NPC::type() { return Mobile::Type::NPC; }
