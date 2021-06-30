// core/parser.hpp -- The command parser! Converts player input into commands that the game can understand.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Inventory;    // defined in world/inventory.hpp


class Parser
{
public:
            Parser();                   // Constructor, sets up the parser.
    void    parse(std::string input);   // Parses input from the player!

private:
    enum class ParserCommand : uint16_t { NONE, CLOSE, DIRECTION, DROP, EQUIP, EQUIPMENT, EXITS, GO, HASH, INVENTORY, LOCK, LOOK, NO, OPEN, SAVE, SPAWN_ITEM, SWEAR, TAKE, TELEPORT,
        TIME, UNEQUIP, UNLOCK, WAIT, WEATHER, XYZZY, YES, QUIT };
    enum class SpecialState : uint8_t { NONE, QUIT_CONFIRM, DISAMBIGUATION };
    enum ItemMatch : uint32_t { NOT_FOUND = UINT_MAX, UNCLEAR = UINT_MAX - 1, VALID = UINT_MAX - 2 };
    enum Target : uint32_t { NONE = UINT_MAX, ITEM_INV = UINT_MAX - 1, ITEM_EQU = UINT_MAX - 2, ITEM_ROOM = UINT_MAX - 3 };

    struct ParserCommandData
    {
        bool            any_length;
        bool            direction_match;
        ParserCommand   command;
        std::string     first_word;
        bool            target_match;
        std::vector<std::string>    words;
    };

    void        add_command(const std::string &text, ParserCommand cmd);    // Adds a command to the parser.
    Direction   parse_direction(const std::string &dir) const;  // Parses a string into a Direction enum.
    uint32_t    parse_item_name(const std::vector<std::string> &input, std::shared_ptr<Inventory> inv); // Attempts to match an item name in the given Inventory.
    void        parse_pcd(const std::string &first_word, const std::vector<std::string> &words, ParserCommandData pcd); // Parses a known command.

    std::vector<ParserCommandData>  m_commands;         // The commands the parser can understand.
    SpecialState                    m_special_state;    // Special parser states, such as waiting for the player to confirm something.
};
