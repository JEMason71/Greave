// actions/doors.h -- Actions involving doors, windows, and other such similar things.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_ACTIONS_DOORS_H_
#define GREAVE_ACTIONS_DOORS_H_

#include <memory>

#include "world/mobile.h"
#include "world/room.h"


class ActionDoors
{
public:
    static bool lock_or_unlock(std::shared_ptr<Mobile> mob, Direction dir, bool unlock, bool confirm, bool silent_fail = false);    // Attempts to lock or unlock a door, with optional messages.
    static bool open_or_close(std::shared_ptr<Mobile> mob, Direction dir, bool open, bool confirm); // Attempts to open or close a door, window, or other openable portal.

private:
    static constexpr float  TIME_CLOSE_DOOR =   2;  // The time taken (in seconds) to close a door.
    static constexpr float  TIME_LOCK_DOOR =    10; // The time taken (in seconds) to lock a door.
    static constexpr float  TIME_OPEN_DOOR =    3;  // The time taken (in seconds) to open a door.
    static constexpr float  TIME_UNLOCK_DOOR =  10; // The time taken (in seconds) to unlock a door.
};

#endif  // GREAVE_ACTIONS_DOORS_H_
