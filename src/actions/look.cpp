// actions/look.cpp -- Look around you. Just look around you.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/look.hpp"
#include "core/core.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
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
        uint32_t room_link = room->link(e);
        if (!room_link || room_link == Room::BLOCKED) continue;
        if (room->link_tag(e, LinkTag::Hidden)) continue;   // Never list hidden exits.
        std::string exit_name = "{c}" + StrX::dir_to_name(e);
        const std::string door_name = room->door_name(e);
        
        if (room_link == Room::UNFINISHED)  // An exit that is due to be finished later.
            exit_name = "{r}(" + StrX::dir_to_name(e) + "){c}";
        else if (room_link == Room::FALSE_ROOM)
        {
            if (room->link_tag(e, LinkTag::KnownLocked)) exit_name += " {m}[locked<>]{c}";
            else exit_name += " {u}[closed<>]{c}";
        }
        else
        {
            const std::shared_ptr<Room> link_room = core()->world()->get_room(room_link);
            if (link_room->tag(RoomTag::Explored) && !link_room->tag(RoomTag::Maze)) exit_name += " {B}(" + link_room->name(true) + "){c}";
            if (room->link_tag(e, LinkTag::KnownLocked)) exit_name += " {m}[locked<>]{c}";
            else if (room->link_tag(e, LinkTag::Openable))
            {
                if (room->link_tag(e, LinkTag::Open)) exit_name += " {u}[open<>]{c}";
                else exit_name += " {u}[closed<>]{c}";
            }
        }

        if (door_name == "door" || door_name == "metal door") StrX::find_and_replace(exit_name, "<>", "");   // Don't specify 'door' if it's just a door.
        else StrX::find_and_replace(exit_name, "<>", " " + door_name);  // Any other 'door' types (windows, grates, etc.) should be specified.

        exits_vec.push_back(exit_name);
    }
    if (exits_vec.size()) core()->message("{0}{g}```Obvious exits: " + StrX::comma_list(exits_vec, StrX::CL_FLAG_USE_AND));

    // Items nearby.
    if (room->inv()->count())
    {
        std::vector<std::string> items_nearby;
        for (unsigned int i = 0; i < room->inv()->count(); i++)
            items_nearby.push_back(room->inv()->get(i)->name() + "{w}");
        core()->message("{0}{g}```Items nearby: {w}" + StrX::comma_list(items_nearby, StrX::CL_FLAG_USE_AND));
    }
}
