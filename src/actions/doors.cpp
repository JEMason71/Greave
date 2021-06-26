// actions/doors.cpp -- Actions involving doors, windows, and other such similar things.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/doors.hpp"
#include "core/core.hpp"
#include "core/mathx.hpp"
#include "core/strx.hpp"
#include "world/mobile.hpp"
#include "world/room.hpp"
#include "world/world.hpp"


// Attempts to open or close a door, window, or other openable portal.
bool ActionDoors::open_or_close(std::shared_ptr<Mobile> mob, Direction dir, bool open)
{
    const uint32_t mob_loc = mob->location();
    //const uint32_t player_loc = core()->world()->player()->location();
    const std::shared_ptr<Room> room = core()->world()->get_room(mob_loc);
    //const bool source_visible = (mob_loc == player_loc);
    //const bool dest_visible = (room->link(dir) == player_loc);
    const bool is_player = (mob->type() == Mobile::Type::PLAYER);
    const bool is_open = room->link_tag(dir, LinkTag::Open);
    const std::string open_close_str = (open ? "open" : "close");
    const std::string open_closed_str = (open ? "open" : "closed");

    if (!room->link_tag(dir, LinkTag::Openable))
    {
        if (is_player) core()->message("{y}That isn't something you can {Y}" + open_close_str + "{y}!");
        // todo: add fail message for NPCs
        return false;
    }

    if (open == is_open)
    {
        if (is_player) core()->message("{y}You can't do that, it's {Y}already " + open_closed_str + "{y}!");
        // todo: add fail message for NPCs
        return false;
    }

    // The lock code has been TEMPORARILY DISABLED while building. It will be re-enabled later, when I add a skeleton key debug item.
    /*
    if (open && room->link_tag(dir, LinkTag::Locked))
    {
        if (is_player) core()->message("{y}You try to open it, but it appears to be {Y}locked tight{y}!");
        // todo: add fail message for NPCs
        return false;
    }
    */

    const std::string door_name = room->door_name(dir);
    if (is_player) core()->message("{u}You {U}" + open_close_str + " the " + door_name + " {u}" + StrX::dir_to_name(dir, StrX::DirNameType::TO_THE) + ".");
    // todo: add open/close messages for NPCs, in both source and destination rooms

    const std::shared_ptr<Room> dest_room = core()->world()->get_room(room->link(dir));
    if (open)
    {
        room->set_link_tag(dir, LinkTag::Open);
        dest_room->set_link_tag(MathX::dir_invert(dir), LinkTag::Open);
    }
    else
    {
        room->clear_link_tag(dir, LinkTag::Open);
        dest_room->clear_link_tag(MathX::dir_invert(dir), LinkTag::Open);
    }

    return true;
}
