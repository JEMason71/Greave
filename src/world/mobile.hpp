// world/mobile.hpp -- The Mobile class defines entities that can move and interact with the game world. Derived classes are used for more specific entities.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


class Mobile
{
public:
    enum class Type : uint8_t { NPC, PLAYER };  // enum used in type() below, to easily check if we're dealing with an NPC or Player when using a Mobile pointer.

                    Mobile();           // Constructor, sets default values.
    uint32_t        location() const;   // Retrieves the location of this Mobile, in the form of a Room ID.
    void            set_location(uint32_t room_id); // Sets the location of this Mobile with a Room ID.
    void            set_location(const std::string &room_id);   // As above, but with a string Room ID.
    virtual Type    type() = 0;         // Mobile should never be instantiated directly, only NPC or Player.

private:
    uint32_t    m_location; // The Room that this Mobile is currently located in.
};
