// actions/look.cpp -- Look around you. Just look around you.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/look.hpp"
#include "core/core.hpp"
#include "core/strx.hpp"
#include "world/mobile.hpp"
#include "world/room.hpp"
#include "world/world.hpp"


// Take a look around at your surroundings.
void ActionLook::look(std::shared_ptr<Mobile> mob)
{
    // We're going to just assume the Mobile specified is the player. If not... we'll look around anyway, from the Mobile's point of view. Why? Because even if it doesn't make much
    // sense right now, you never know, this code might be used in the future for some sort of "see through the target's eyes" spell or something.
    const std::shared_ptr<Room> room = core()->world()->get_room(mob->location());

    //const bool can_see = (room->light() >= Room::LIGHT_VISIBLE);
    const bool can_see = true;
    if (!can_see)
    {
        core()->message("{U}Darkness");
        core()->message("{0}```{u}It is {B}pitch black{u}, and you can see {B}nothing{u}. You are likely to be eaten by a grue.");
        core()->message("{0}{u}It's so {B}dark{u}, you can't see where the exits are!");
        return;
    }

    room->set_tag(RoomTag::Explored);

    // Room name and description.
    core()->message("{G}" + room->name());
    core()->message("{0}```" + room->desc());

    // Obvious exits.
    std::vector<std::string> exits_vec;
    for (unsigned int e = 0; e < Room::ROOM_LINKS_MAX; e++)
    {
        std::string exit_name;
        uint32_t room_link = room->link(e);
        if (!room_link || room_link == Room::BLOCKED) continue;
        if (room->link_tag(e, LinkTag::Hidden)) continue;   // Never list hidden exits.
        exit_name = "{c}" + StrX::dir_to_name(e);
        
        if (room_link == Room::UNFINISHED)  // An exit that is due to be finished later.
            exit_name = "{r}(" + StrX::dir_to_name(e) + "){c}";
        else if (room_link == Room::FALSE_ROOM) exit_name += " {u}[locked]{c}";
        else
        {
            const std::shared_ptr<Room> link_room = core()->world()->get_room(room_link);
            const bool is_window = room->link_tag(e, LinkTag::Window);
            if (link_room->tag(RoomTag::Explored) && !link_room->tag(RoomTag::Maze)) exit_name += " {B}(" + link_room->name(true) + "){c}";
            if (room->link_tag(e, LinkTag::Locked)) exit_name += (is_window ? " {u}[locked window]{c}" : " {u}[locked]{c}");
            else if (room->link_tag(e, LinkTag::Openable))
            {
                if (room->link_tag(e, LinkTag::Open)) exit_name += (is_window ? " {u}[open window]{c}" : " {u}[open]{c}");
                else exit_name += (is_window ? " {u}[closed window]{c}" : " {u}[closed]{c}");
            }
        }

        exits_vec.push_back(exit_name);
    }

    if (exits_vec.size()) core()->message("{0}{g}Obvious exits: " + StrX::comma_list(exits_vec, StrX::CL_FLAG_USE_AND));
}
