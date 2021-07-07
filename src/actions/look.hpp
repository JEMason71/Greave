// actions/look.hpp -- Look around you. Just look around you.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Item;                     // defined in world/item.hpp
class Mobile;                   // defined in world/mobile.hpp
enum ParserTarget : uint8_t;    // defined in core/parser.hpp


class ActionLook
{
public:
    static void examine(std::shared_ptr<Mobile> mob, ParserTarget target_type, size_t target);  // Examines an Item or Mobile.
    static void look(std::shared_ptr<Mobile> mob);          // Take a look around at your surroundings.
    static void obvious_exits(std::shared_ptr<Mobile> mob, bool indent);    // Lists the exits from this area.

private:
    static void examine_item(std::shared_ptr<Mobile> mob, std::shared_ptr<Item> target);        // Examines an Item.
    static void examine_mobile(std::shared_ptr<Mobile> mob, std::shared_ptr<Mobile> target);    // Examines a Mobile.
};
