// actions/cheat.h -- Cheating, debugging and testing commands.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_ACTIONS_CHEAT_H_
#define GREAVE_ACTIONS_CHEAT_H_

#include <cstddef>

#include <string>


namespace greave {
namespace cheat {

void    add_money(int32_t amount);      // Adds money to the player's wallet.
void    colours();                      // Displays all the colours!
void    heal(size_t target);            // Heals the player or an NPC.
void    spawn_item(std::string item);   // Attempts to spawn an item.
void    spawn_mobile(std::string mob);  // Attempts to spawn a mobile.
void    teleport(std::string dest);     // Attemtps to teleport to another room.

}       // namespace cheat
}       // namespace greave
#endif  // GREAVE_ACTIONS_CHEAT_H_
