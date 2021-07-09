// core/parser.hpp -- The command parser! Converts player input into commands that the game can understand.
// Copyright (c) 2021 Raine "Gravecat" Simmons and the Greave contributors. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Inventory;                // defined in world/inventory.hpp
enum class Direction : uint8_t; // defined in world/room.hpp


enum ParserTarget : uint8_t { TARGET_NONE = 0, TARGET_EQUIPMENT = 1, TARGET_INVENTORY = 2, TARGET_MOBILE = 4, TARGET_ROOM = 8, TARGET_UNCLEAR = 16 };

class Parser
{
public:
            Parser();                   // Constructor, sets up the parser.
    void    confirm_message();          // Tells the player how to confirm a command.
    void    parse(std::string input);   // Parses input from the player!

private:
    enum class ParserCommand : uint16_t { NONE, ADD_MONEY, ATTACK, CLOSE, DIRECTION, DROP, EQUIP, EQUIPMENT, EXAMINE, EXITS, GO, HASH, HELP, INVENTORY, LOCK, LOOK, MIXUP, MIXUP_BIG,
        NO, OPEN, SAVE, SCORE, SPAWN_ITEM, SPAWN_MOBILE, STANCE, SWEAR, TAKE, TELEPORT, TIME, UNEQUIP, UNLOCK, WAIT, WEATHER, XYZZY, YES, QUIT };
    enum class SpecialState : uint8_t { NONE, QUIT_CONFIRM, DISAMBIGUATION };

    struct ParserCommandData
    {
        bool                        any_length;
        bool                        direction_match;
        ParserCommand               command;
        std::string                 first_word;
        bool                        target_match;
        std::vector<std::string>    words;
    };

    struct ParserSearchResult
    {
        int             score;
        std::string     name, name_np;
        uint32_t        parser_id;
        uint32_t        target;
        ParserTarget    type;
    };

    void                add_command(const std::string &text, ParserCommand cmd);    // Adds a command to the parser.
    Direction           parse_direction(const std::string &dir) const;  // Parses a string into a Direction enum.
    int32_t             parse_int(const std::string &s);    // Wrapper function to check for out of range values.
    ParserSearchResult  parse_target(const std::vector<std::string> &input, ParserTarget target);   // Attempts to match a name to a given target.
    void                parse_pcd(const std::string &first_word, const std::vector<std::string> &words, ParserCommandData pcd, bool confirm);   // Parses a known command.

    std::vector<ParserCommandData>  m_commands;         // The commands the parser can understand.
    std::string                     m_last_input;       // The last raw input from the player.
    SpecialState                    m_special_state;    // Special parser states, such as waiting for the player to confirm something.
};
