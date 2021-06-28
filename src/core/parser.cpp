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
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/mobile.hpp"
#include "world/room.hpp"
#include "world/world.hpp"


// Constructor, sets default values.
Parser::Parser() : m_debug_parser(std::make_shared<DebugParser>()), m_disambiguation_parsing(false), m_special_state(SpecialState::NONE) { }

// Checks if a directional command is valid, returns a Direction if so.
Direction Parser::direction_command(const std::vector<std::string> &input, std::string command_override) const
{
    if (!command_override.size()) command_override = input.at(0);
    if (input.size() < 2)
    {
        core()->message("{y}Please specify a {Y}direction {y}to " + command_override + ".");
        return Direction::NONE;
    }
    const Direction dir = parse_direction(input.at(1));
    if (dir == Direction::NONE) core()->message("{y}Please specify a {Y}compass direction {y}to " + command_override + ".");
    return dir;
}

// Parses input from the player!
void Parser::parse(std::string input)
{
    if (!input.size()) return;
    std::vector<std::string> words = StrX::string_explode(StrX::str_tolower(input), " ");
    if (!words.size()) return;  // This is incredibly unlikely, possibly impossible, but it can't hurt to check.

    std::string first_word = words.at(0);
    if (!first_word.size()) return; // Also incredibly unlikely, but let's be safe.
    const std::shared_ptr<Mobile> player = core()->world()->player();
    const std::shared_ptr<Room> room = core()->world()->get_room(player->location());

    // Hide the disambiguation data temporarily. It'll be restored below, if a command cannot be parsed.
    std::vector<uint32_t> disambiguation_copy = m_disambiguation;
    std::string disambiguation_command_copy = m_disambiguation_command;

    // Check if we're parsing a disambiguation.
    if (first_word[0] == '}')
    {
        first_word = words.at(0) = first_word.substr(1);
        m_disambiguation_parsing = true;
    }
    else
    {
        m_disambiguation_parsing = false;
        m_disambiguation.clear();
        m_disambiguation_command.clear();
    }

    switch (m_special_state)
    {
        case SpecialState::NONE: break;
        case SpecialState::QUIT_CONFIRM:
            parse_quit_confirm(input, first_word);
            return;
    }

    // Used for commands that target items.
    auto item_target = [this, &words, &player, &room](bool in_inventory, std::string command, const std::string &error_specify, std::string error_not_found) -> uint32_t {
        if (words.size() < 2)
        {
            core()->message(error_specify);
            return ItemMatch::NOT_FOUND;
        }
        std::vector<std::string> target = words;
        target.erase(target.begin());
        const uint32_t inv_pos = parse_item_name(target, (in_inventory ? player->inv() : room->inv()));
        if (inv_pos == ItemMatch::NOT_FOUND)
        {
            if (!m_disambiguation_parsing)
            {
                StrX::find_and_replace(error_not_found, "<target>", StrX::collapse_vector(target));
                core()->message(error_not_found);
                return ItemMatch::NOT_FOUND;
            }
            return ItemMatch::DO_NOTHING;
        }
        else if (inv_pos == ItemMatch::UNCLEAR) // Disambiguation, error message already handled by parse_item_name().
        {
            m_disambiguation_command = command;
            return ItemMatch::NOT_FOUND;
        }
        return inv_pos;
    };


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
        const Direction dir = direction_command(words);
        if (dir != Direction::NONE) ActionDoors::open_or_close(player, dir, (first_word == "open"));
        return;
    }

    // Attempt to lock or unlock a door.
    if (first_word == "lock" || first_word == "unlock")
    {
        const Direction dir = direction_command(words);
        if (dir != Direction::NONE) ActionDoors::lock_or_unlock(player, dir, (first_word == "unlock"));
        return;
    }

    // Movement to nearby Rooms.
    const Direction dir_cmd = parse_direction(first_word);
    if (dir_cmd != Direction::NONE)
    {
        ActionTravel::travel(player, dir_cmd);
        return;
    }
    if (first_word == "go" || first_word == "travel" || first_word == "walk" || first_word == "run" || first_word == "move")
    {
        const Direction dir = direction_command(words, "travel");
        if (dir != Direction::NONE) ActionTravel::travel(player, dir);
        return;
    }


    /*********************************
     * Item and inventory management *
     *********************************/

    if (first_word == "inventory" || first_word == "invent" || first_word == "inv" || first_word == "i")
    {
        ActionInventory::check_inventory(player);
        return;
    }

    if (first_word == "take" || first_word == "get")
    {
        const uint32_t item_pos = item_target(false, "get {}", "{y}Please specify {Y}what you want to take{y}.", "{y}You don't see {Y}<target>{y} here.");
        if (item_pos == ItemMatch::NOT_FOUND) return;
        else if (item_pos != ItemMatch::DO_NOTHING)
        {
            ActionInventory::take(player, item_pos);
            return;
        }
    }

    if (first_word == "drop")
    {
        const uint32_t item_pos = item_target(true, "drop {}", "{y}Please specify {Y}what you want to drop{y}.", "{y}You don't seem to be carrying {Y}<target>{y}.");
        if (item_pos == ItemMatch::NOT_FOUND) return;
        else if (item_pos != ItemMatch::DO_NOTHING)
        {
            ActionInventory::drop(player, item_pos);
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


    /******************
     * Disambiguation *
     ******************/

    // Restore the copies of the data we made near the start of this function.
    m_disambiguation = disambiguation_copy;
    m_disambiguation_command = disambiguation_command_copy;

    // Okay, so the player just typed "get sword" and the game said, "I'm not sure which one, did you mean red sword or blue sword?"
    // Then the player typed "red". That's obviously not a command we recognize... but let's try to be smart.
    if (m_disambiguation.size() && m_disambiguation_command.size())
    {
        if (StrX::find_and_replace(m_disambiguation_command, "{}", StrX::collapse_vector(words)))
        {
            parse("}" + m_disambiguation_command);
            return;
        }
        else
        {
            m_disambiguation.clear();
            m_disambiguation_command.clear();
        }
    }

    core()->message("{y}I'm sorry, I don't understand.");
}

// Parses a string into a Direction enum.
Direction Parser::parse_direction(const std::string &dir) const
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

// Attempts to match an item name in the given Inventory.
uint32_t Parser::parse_item_name(const std::vector<std::string> &input, std::shared_ptr<Inventory> inv)
{
    if (!input.size() || ! inv->count()) return ItemMatch::NOT_FOUND;   // Don't even *try* with blank inputs, or empty inventories.

    const uint32_t inv_size = inv->count();
    std::vector<unsigned int> item_scores(inv_size);
    for (unsigned int i = 0; i < inv_size; i++)
    {
        if (m_disambiguation_parsing)
        {
            bool good = false;
            for (auto d : m_disambiguation)
            {
                if (i == d)
                {
                    good = true;
                    break;
                }
            }
            if (!good) continue;
        }

        const std::shared_ptr<Item> item = inv->get(i);
        if (input.size() == 1 && input.at(0) == StrX::itoh(item->hex_id(), 3)) return i;    // If the hex ID matches, that's an easy one.

        // Score each object by matching words in its name.
        const std::string stripped_name_lowercase = StrX::str_tolower(StrX::strip_ansi(item->name()));
        std::vector<std::string> name_words = StrX::string_explode(stripped_name_lowercase, " ");
        const std::string collapsed_input = StrX::collapse_vector(input);
        unsigned int score = 0;
        if (stripped_name_lowercase == collapsed_input) score = 100;
        else
        {
            for (unsigned int j = 0; j < input.size(); j++)
                for (unsigned int k = 0; k < name_words.size(); k++)
                    if (input.at(j) == name_words.at(k)) score++;
        }
        item_scores.at(i) = score;
    }
    m_disambiguation.clear();

    // Determine the highest score.
    unsigned int highest_score = 0;
    for (unsigned int i = 0; i < inv_size; i++)
        if (item_scores.at(i) > highest_score) highest_score = item_scores.at(i);
    
    // No matches at all?
    if (!highest_score)
    {
        m_disambiguation.clear();
        return ItemMatch::NOT_FOUND;
    }

    // Now loop again, see how many candidates we have.
    for (unsigned int i = 0; i < inv_size; i++)
        if (item_scores.at(i) == highest_score) m_disambiguation.push_back(i);
    
    // If we just have one, great! That was easy!
    if (m_disambiguation.size() == 1)
    {
        const uint32_t choice = m_disambiguation.at(0);
        m_disambiguation.clear();
        return choice;
    }

    // If not...
    std::string disambig = "{c}I'm not sure which one you mean! Did you mean ";
    std::vector<std::string> candidate_names;
    for (auto c : m_disambiguation)
        candidate_names.push_back("{C}" + inv->get(c)->name() + " {B}{" + StrX::itoh(inv->get(c)->hex_id(), 3) + "}{c}");
    disambig += StrX::comma_list(candidate_names) + "?";
    core()->message(disambig);

    return ItemMatch::UNCLEAR;
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
