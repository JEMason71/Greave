// core/parser.hpp -- The command parser! Converts player input into commands that the game can understand.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class DebugParser;  // defined in debug/parser.hpp


class Parser
{
public:
                Parser();   // Constructor, sets default values.
    void        parse(std::string input);   // Parses input from the player!
    Direction   parse_direction(const std::string &dir) const;  // Parses a string into a Direction enum.

private:
    enum class SpecialState : uint8_t { NONE, QUIT_CONFIRM };

    std::shared_ptr<DebugParser>    m_debug_parser;     // The debug parser, which handles dev/testing/cheat commands.
    SpecialState                    m_special_state;    // Special parser states, such as waiting for the player to confirm something.

    Direction   direction_command(const std::vector<std::string> &input, std::string command_override = "") const;  // Checks if a directional command is valid.
    void        parse_quit_confirm(const std::string &input, const std::string &first_word);   // Parses input when QUIT_CONFIRM state is active.
};
