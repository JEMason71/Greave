// actions/look.cpp -- Look around you. Just look around you.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/look.hpp"
#include "core/core.hpp"
#include "world/mobile.hpp"
#include "world/room.hpp"
#include "world/world.hpp"


// Take a look around at your surroundings.
void ActionLook::look(std::shared_ptr<Mobile> mob)
{
    // We're going to just assume the Mobile specified is the player. If not... we'll look around anyway, from the Mobile's point of view. Why? Because even if it doesn't make much
    // sense right now, you never know, this code might be used in the future for some sort of "see through the target's eyes" spell or something.
    const std::shared_ptr<Room> room = core()->world()->get_room(mob->location());

    core()->message("{G}" + room->name());
    core()->message("{0}" + room->desc());
}
