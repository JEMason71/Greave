// actions/cheat.h -- Cheating, debugging and testing commands.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_ACTIONS_CHEAT_H_
#define GREAVE_ACTIONS_CHEAT_H_

#include <cstddef>

#include <string>


namespace greave {

class ActionCheat
{
public:
    static void add_money(int32_t amount);      // Adds money to the player's wallet.
    static void colours();                      // Displays all the colours!
    static void heal(size_t target);            // Heals the player or an NPC.
    static void spawn_item(std::string item);   // Attempts to spawn an item.
    static void spawn_mobile(std::string mob);  // Attempts to spawn a mobile.
    static void teleport(std::string dest);     // Attemtps to teleport to another room.
};

}       // namespace greave
#endif  // GREAVE_ACTIONS_CHEAT_H_
