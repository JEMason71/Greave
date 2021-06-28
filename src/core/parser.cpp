// core/parser.cpp -- The command parser! Converts player input into commands that the game can understand.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/doors.hpp"
#include "actions/inventory.hpp"
#include "actions/look.hpp"
#include "actions/travel.hpp"
#include "core/core.hpp"
#include "core/parser.hpp"
#include "debug/parser.hpp"
#include "core/strx.hpp"
#include "world/mobile.hpp"
#include "world/world.hpp"


// Constructor, sets default values.
Parser::Parser() : m_debug_parser(std::make_shared<DebugParser>()), m_special_state(SpecialState::NONE) { }

// Parses input from the player!
void Parser::parse(std::string input)
{
    if (!input.size()) return;
    std::vector<std::string> words = StrX::string_explode(StrX::str_tolower(input), " ");
    if (!words.size()) return;  // This is incredibly unlikely, possibly impossible, but it can't hurt to check.

    const std::string first_word = words.at(0);
    if (!first_word.size()) return; // Also incredibly unlikely, but let's be safe.
    const std::shared_ptr<Mobile> player = core()->world()->player();

    switch (m_special_state)
    {
        case SpecialState::NONE: break;
        case SpecialState::QUIT_CONFIRM:
            parse_quit_confirm(input, first_word);
            return;
    }


    /*****************
     * Meta commands *
     *****************/

    // 'quit' and 'exit' commands both exit the game, but require confirmation.
    if (first_word == "quit" || first_word == "exit")
    {
        core()->message("{R}Are you sure you want to quit? {M}Your game will not be saved. {R}Type {C}yes {R}to confirm.");
        m_special_state = SpecialState::QUIT_CONFIRM;
        return;
    }

    // 'save' command, to... save the game.
    if (first_word == "save") { core()->save(); return; }


    /****************************
     * Basic world interactions *
     ****************************/

    // Look around you. Just look around you.
    if (first_word == "look" || first_word == "l") { ActionLook::look(player); return; }

    // Atempt to open or close a door or something else openable.
    if (first_word == "open" || first_word == "close")
    {
        if (words.size() < 2)
        {
            core()->message("{y}Please specify a {Y}direction {y}to " + first_word + ".");
            return;
        }
        const bool open = (first_word == "open");
        const Direction dir = parse_direction(words.at(1));
        if (dir == Direction::NONE)
        {
            core()->message("{y}Please specify a {Y}compass direction {y}to " + first_word + ".");
            return;
        }
        ActionDoors::open_or_close(player, dir, open);
        return;
    }

    // Movement to nearby Rooms.
    const Direction dir_cmd = parse_direction(first_word);
    if (dir_cmd != Direction::NONE)
    {
        ActionTravel::travel(player, dir_cmd);
        return;
    }
    else if (first_word == "go" || first_word == "travel" || first_word == "walk" || first_word == "run" || first_word == "move")
    {
        if (words.size() < 2)
        {
            core()->message("{y}Please specify a {Y}direction {y}to travel.");
            return;
        }
        const Direction dir = parse_direction(words.at(1));
        if (dir == Direction::NONE)
        {
            core()->message("{y}Please specify a {Y}compass direction {y}to travel.");
            return;
        }
        ActionTravel::travel(player, dir);
        return;
    }


    /*********************************
     * Item and inventory management *
     *********************************/

    else if (first_word == "inventory" || first_word == "invent" || first_word == "inv" || first_word == "i")
    {
        ActionInventory::check_inventory(player);
        return;
    }

    else if (first_word == "take" || first_word == "get")
    {
        if (words.size() < 2)
        {
            core()->message("{y}Please specify {Y}what you want to get{y}.");
            return;
        }
        if (words.at(1) == "inventory" || words.at(1) == "invent" || words.at(1) == "inv" || words.at(1) == "i")
        {
            ActionInventory::check_inventory(player);
            return;
        }
    }


    /************************************
     * Super secret dev/debug commands! *
     ************************************/

    if (first_word[0] == '#')
    {
        m_debug_parser->parse(words);
        return;
    }


    /*********
     * Fluff *
     *********/

    // Other words we can basically ignore.
    if (first_word == "yes" || first_word == "no")
    {
        core()->message("{y}That was a rhetorical question.");
        return;
    }
    if (first_word == "xyzzy" || first_word == "frotz" || first_word == "plugh" || first_word == "plover")
    {
        core()->message("{u}A hollow voice says, {m}\"Fool.\"");
        return;
    }
    if (first_word == "fuck" || first_word == "shit" || first_word == "piss" || first_word == "bastard")
    {
        core()->message("{y}Real adventurers do not use such language.");
        return;
    }

    core()->message("{y}I'm sorry, I don't understand.");
}

// Parses a string into a Direction enum.
Direction Parser::parse_direction(const std::string &dir)
{
    if (dir == "north" || dir == "n") return Direction::NORTH;
    else if (dir == "northeast" || dir == "ne") return Direction::NORTHEAST;
    else if (dir == "east" || dir == "e") return Direction::EAST;
    else if (dir == "southeast" || dir == "se") return Direction::SOUTHEAST;
    else if (dir == "south" || dir == "s") return Direction::SOUTH;
    else if (dir == "southwest" || dir == "sw") return Direction::SOUTHWEST;
    else if (dir == "west" || dir == "w") return Direction::WEST;
    else if (dir == "northwest" || dir == "nw") return Direction::NORTHWEST;
    else if (dir == "up" || dir == "u") return Direction::UP;
    else if (dir == "down" || dir == "d") return Direction::DOWN;
    else return Direction::NONE;
}

// Parses input when QUIT_CONFIRM state is active; the game is waiting for confirmation that the player wants to quit.
void Parser::parse_quit_confirm(const std::string &input, const std::string &first_word)
{
    if (first_word == "yes")
    {
        core()->cleanup();
        exit(EXIT_SUCCESS);
    }
    else if (first_word == "no")
    {
        core()->message("{y}Very well. Your adventure continues.");
        m_special_state = SpecialState::NONE;
        return;
    }
    else
    {
        m_special_state = SpecialState::NONE;
        parse(input);
    }
}
