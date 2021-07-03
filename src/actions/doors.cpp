// actions/doors.cpp -- Actions involving doors, windows, and other such similar things.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/doors.hpp"
#include "core/core.hpp"
#include "core/mathx.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/mobile.hpp"
#include "world/room.hpp"
#include "world/world.hpp"


const float ActionDoors::TIME_CLOSE_DOOR =  2.0f;   // The time taken (in seconds) to close a door.
const float ActionDoors::TIME_LOCK_DOOR =   10.0f;  // The time taken (in seconds) to lock a door.
const float ActionDoors::TIME_OPEN_DOOR =   3.0f;   // The time taken (in seconds) to open a door.
const float ActionDoors::TIME_UNLOCK_DOOR = 10.0f;  // The time taken (in seconds) to unlock a door.


// Attempts to lock or unlock a door, with optional messages.
bool ActionDoors::lock_or_unlock(std::shared_ptr<Mobile> mob, Direction dir, bool unlock, bool silent_fail)
{
    const uint32_t mob_loc = mob->location();
    const std::shared_ptr<Room> room = core()->world()->get_room(mob_loc);
    const bool is_player = mob->is_player();
    const bool is_unlocked = !room->link_tag(dir, LinkTag::Locked);
    const std::string lock_unlock_str = (unlock ? "unlock" : "lock");
    const std::string locked_unlocked_str = (unlock ? "unlocked" : "locked");

    if (!room->link_tag(dir, LinkTag::Lockable))
    {
        if (!silent_fail)
        {
            if (is_player) core()->message("{y}That isn't something you can {Y}" + lock_unlock_str + "{y}!");
            // todo: add fail message for NPCs
        }
        return false;
    }

    if (unlock == is_unlocked)
    {
        if (!silent_fail)
        {
            if (is_player) core()->message("{y}You can't do that, it's {Y}already " + locked_unlocked_str + "{y}!");
            // todo: add fail message for NPCs
            return false;
        }
        return false;
    }

    std::shared_ptr<Item> correct_key = nullptr;
    for (unsigned int i = 0; i < mob->inv()->count(); i++)
    {
        const std::shared_ptr<Item> item = mob->inv()->get(i);
        if (item->type() != ItemType::KEY) continue;
        if (room->key_can_unlock(item, dir))
        {
            correct_key = item;
            break;
        }
    }
    if (!correct_key)
    {
        if (!silent_fail)
        {
            if (is_player) core()->message("{y}You can't do that, you don't have {Y}the correct key{y}.");
            // todo: add fail message for NPCs
        }
        return false;
    }

    const std::string door_name = room->door_name(dir);

    // If we're locking a door, make sure it's closed first.
    if (!unlock && room->link_tag(dir, LinkTag::Open))
    {
        if (is_player) core()->message("{m}(first closing the " + door_name + ")");
        if (!open_or_close(mob, dir, false)) return false;
    }

    const float time_taken = (unlock ? TIME_UNLOCK_DOOR : TIME_LOCK_DOOR);
    if (!mob->pass_time(time_taken))
    {
        core()->message("{R}You are interrupted while attempting to " + lock_unlock_str + " the " + door_name + "!");
        return false;
    }

    if (is_player) core()->message("{u}You {U}" + lock_unlock_str + " {u}the " + door_name + " " + StrX::dir_to_name(dir, StrX::DirNameType::TO_THE) + " with your {U}" +
        correct_key->name() + "{u}.");
    // todo: add lock/unlock messages for NPCs, in both source and destination rooms

    const std::shared_ptr<Room> dest_room = core()->world()->get_room(room->link(dir));
    const Direction dir_invert = MathX::dir_invert(dir);
    if (unlock)
    {
        room->set_link_tag(dir, LinkTag::Unlocked);
        room->clear_link_tag(dir, LinkTag::Locked);
        room->clear_link_tag(dir, LinkTag::KnownLocked);
        dest_room->set_link_tag(dir_invert, LinkTag::Unlocked);
        dest_room->clear_link_tag(dir_invert, LinkTag::Locked);
        dest_room->clear_link_tag(dir_invert, LinkTag::KnownLocked);
    }
    else
    {
        room->set_link_tag(dir, LinkTag::Locked);
        room->clear_link_tag(dir, LinkTag::Unlocked);
        dest_room->set_link_tag(dir_invert, LinkTag::Locked);
        dest_room->clear_link_tag(dir_invert, LinkTag::Unlocked);
        if (is_player)
        {
            room->set_link_tag(dir, LinkTag::KnownLocked);
            dest_room->set_link_tag(dir_invert, LinkTag::KnownLocked);
        }
    }

    return true;
}

// Attempts to open or close a door, window, or other openable portal.
bool ActionDoors::open_or_close(std::shared_ptr<Mobile> mob, Direction dir, bool open)
{
    const uint32_t mob_loc = mob->location();
    const std::shared_ptr<Room> room = core()->world()->get_room(mob_loc);
    const bool is_player = mob->is_player();
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

    if (open && room->link_tag(dir, LinkTag::Locked))
    {
        bool unlock_attempt = lock_or_unlock(mob, dir, true, true);
        if (!unlock_attempt)
        {
            if (is_player)
            {
                core()->message("{y}You try to open it, but it appears to be {Y}locked tight{y}!");
                room->set_link_tag(dir, LinkTag::KnownLocked);
                if (!room->fake_link(dir))
                {
                    const std::shared_ptr<Room> dest_room = core()->world()->get_room(room->link(dir));
                    dest_room->set_link_tag(MathX::dir_invert(dir), LinkTag::KnownLocked);
                }
            }
            // todo: add fail message for NPCs
            return false;
        }
    }

    const std::string door_name = room->door_name(dir);

    const float time_taken = (open ? TIME_OPEN_DOOR : TIME_CLOSE_DOOR);
    if (!mob->pass_time(time_taken))
    {
        core()->message("{R}You are interrupted while trying to " + open_close_str + " the " + door_name + "!");
        return false;
    }

    if (is_player) core()->message("{u}You {U}" + open_close_str + " {u}the " + door_name + " " + StrX::dir_to_name(dir, StrX::DirNameType::TO_THE) + ".");
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
