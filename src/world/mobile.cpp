// world/mobile.cpp -- The Mobile class defines entities that can move and interact with the game world. Derived classes are used for more specific entities.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "world/mobile.hpp"


// Constructor, sets default values.
Mobile::Mobile() : m_location(0) { }

// Retrieves the location of this Mobile, in the form of a Room ID.
uint32_t Mobile::location() const { return m_location; }

// Sets the location of this Mobile with a Room ID.
void Mobile::set_location(uint32_t room_id) { m_location = room_id; }
