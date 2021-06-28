// core/parser.hpp -- The command parser! Converts player input into commands that the game can understand.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class DebugParser;  // defined in debug/parser.hpp
class Inventory;    // defined in world/inventory.hpp


class Parser
{
public:
                Parser();   // Constructor, sets default values.
    void        parse(std::string input);   // Parses input from the player!

private:
    enum class SpecialState : uint8_t { NONE, QUIT_CONFIRM };
    enum ItemMatch : uint32_t { NOT_FOUND = UINT_MAX, UNCLEAR = UINT_MAX - 1, DO_NOTHING = UINT_MAX - 2 };

    std::shared_ptr<DebugParser>    m_debug_parser;     // The debug parser, which handles dev/testing/cheat commands.
    std::vector<uint32_t>           m_disambiguation;   // Used when the player specifies a target and the parser isn't sure which thing they mean.
    std::string                     m_disambiguation_command;   // Attempts to remember the player's command when they enter an uncertain target.
    bool                            m_disambiguation_parsing;   // Are we currently parsing a reply from disambiguation?
    SpecialState                    m_special_state;    // Special parser states, such as waiting for the player to confirm something.

    Direction   direction_command(const std::vector<std::string> &input, std::string command_override = "") const;  // Checks if a directional command is valid.
    Direction   parse_direction(const std::string &dir) const;  // Parses a string into a Direction enum.
    uint32_t    parse_item_name(const std::vector<std::string> &input, std::shared_ptr<Inventory> inv); // Attempts to match an item name in the given Inventory.
    void        parse_quit_confirm(const std::string &input, const std::string &first_word);   // Parses input when QUIT_CONFIRM state is active.
};
