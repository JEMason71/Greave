// actions/doors.hpp -- Actions involving doors, windows, and other such similar things.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Mobile;   // defined in world/mobile.hpp


class ActionDoors
{
public:
    static bool lock_or_unlock(std::shared_ptr<Mobile> mob, Direction dir, bool unlock, bool silent_fail = false);  // Attempts to lock or unlock a door, with optional messages.
    static bool open_or_close(std::shared_ptr<Mobile> mob, Direction dir, bool open);   // Attempts to open or close a door, window, or other openable portal.
};
