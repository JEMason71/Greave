// actions/travel.cpp -- Actions allowing the player and NPCs to move around the game world.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/look.hpp"
#include "actions/travel.hpp"
#include "core/core.hpp"
#include "core/strx.hpp"
#include "world/mobile.hpp"
#include "world/room.hpp"
#include "world/world.hpp"


// Attempts to move from one Room to another.
bool ActionTravel::travel(std::shared_ptr<Mobile> mob, Direction dir)
{
    const uint32_t mob_loc = mob->location();
    const std::shared_ptr<Room> room = core()->world()->get_room(mob_loc);
    const bool is_player = (mob->type() == Mobile::Type::PLAYER);

    if (!room->link(dir))
    {
        if (is_player) core()->message("{y}You cannot travel {Y}" + StrX::dir_to_name(dir, StrX::DirNameType::TO_THE_ALT) + "{y}.");
        // todo: NPC failure messages
        return false;
    }

    if (room->link_tag(dir, LinkTag::Openable) && !room->link_tag(dir, LinkTag::Open))
    {
        if (is_player) core()->message("{y}Your passage " + StrX::dir_to_name(dir, StrX::DirNameType::TO_THE) + " {Y}is blocked{y}.");
        // todo: NPC failure messages
        // todo: auto open unlocked doors
        return false;
    }

    mob->set_location(room->link(dir));
    if (is_player) ActionLook::look(mob);
    return true;
}
