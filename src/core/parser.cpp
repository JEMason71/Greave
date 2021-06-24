// core/parser.cpp -- The command parser! Converts player input into commands that the game can understand.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/look.hpp"
#include "core/core.hpp"
#include "core/parser.hpp"
#include "core/strx.hpp"
#include "world/mobile.hpp"
#include "world/world.hpp"


// Constructor, sets default values.
Parser::Parser() : m_special_state(SpecialState::NONE) { }

// Parses input from the player!
void Parser::parse(std::string input)
{
    if (!input.size()) return;
    std::vector<std::string> words = StrX::string_explode(StrX::str_tolower(input), " ");
    if (!words.size()) return;  // This is incredibly unlikely, possibly impossible, but it can't hurt to check.

    const std::string first_word = words.at(0);
    const std::shared_ptr<Mobile> player = core()->world()->player();

    switch (m_special_state)
    {
        case SpecialState::NONE: break;
        case SpecialState::QUIT_CONFIRM:
            parse_quit_confirm(input, first_word);
            return;
    }

    // 'quit' and 'exit' commands both exit the game, but require confirmation.
    if (first_word == "quit" || first_word == "exit")
    {
        core()->message("{R}Are you sure you want to quit? Type {C}yes {R}to confirm.");
        m_special_state = SpecialState::QUIT_CONFIRM;
        return;
    }

    // Look around you. Just look around you.
    if (first_word == "look" || first_word == "l")
    {
        ActionLook::look(player);
        return;
    }

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
