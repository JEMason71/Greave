// debug/parser.cpp -- The debug parser, handles debug/testing/cheat commands starting with #.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/look.hpp"
#include "core/core.hpp"
#include "core/strx.hpp"
#include "debug/parser.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/player.hpp"
#include "world/world.hpp"


// Parses debug commands.
void DebugParser::parse(std::vector<std::string> words)
{
    const std::string first_word = words.at(0);

    // Hashes a string into an integer.
    if (first_word == "#hash")
    {
        if (words.size() < 2)
        {
            core()->message("{y}Please specify a {Y}string to hash{y}.");
            return;
        }
        const std::string hash_word = StrX::str_toupper(words.at(1));
        core()->message("{G}" + hash_word + " {g}hashes to {G}" + std::to_string(StrX::hash(hash_word)) + "{g}.");
        return;
    }

    // Teleports to another room.
    if (first_word == "#tp")
    {
        if (words.size() < 2)
        {
            core()->message("{y}Please specify a {Y}teleport destination{y}.");
            return;
        }
        const std::string target = StrX::str_toupper(words.at(1));
        if (core()->world()->room_exists(target))
        {
            core()->message("{U}The world around you {M}s{C}h{M}i{C}m{M}m{C}e{M}r{C}s{U}!");
            core()->world()->player()->set_location(StrX::hash(target));
            ActionLook::look(core()->world()->player());
        }
        else core()->message("{R}" + target + " {y}is not a valid room ID.");
        return;
    }

    // Spawns an item.
    if (first_word == "#si")
    {
        if (words.size() < 2)
        {
            core()->message("{y}Please specify an {Y}item ID{y}.");
            return;
        }
        const std::string item_id = StrX::str_toupper(words.at(1));
        if (core()->world()->item_exists(item_id))
        {
            const std::shared_ptr<Item> new_item = core()->world()->get_item(item_id);
            core()->message("{C}You use the power of {R}w{Y}i{G}s{U}h{C}f{M}u{R}l {Y}t{G}h{U}i{C}n{M}k{R}i{Y}n{G}g {C}to bring " + new_item->name() + " {C}into the world!");
            core()->world()->player()->inv()->add_item(new_item);
        }
        else core()->message("{R}" + item_id + " {y}is not a valid item ID.");
        return;
    }

    core()->message("{y}I'm sorry, I don't understand.");
}
