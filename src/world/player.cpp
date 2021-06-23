// world/player.cpp -- The Player class is derived from Mobile, and defines the player character in the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "world/player.hpp"


// Returns Mobile::Type::Player, so we can check what type of Mobile this is when using a Mobile pointer.
Mobile::Type Player::type() { return Mobile::Type::PLAYER; }
