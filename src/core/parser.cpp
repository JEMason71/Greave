// core/parser.cpp -- The command parser! Converts player input into commands that the game can understand.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/cheat.hpp"
#include "actions/doors.hpp"
#include "actions/inventory.hpp"
#include "actions/look.hpp"
#include "actions/travel.hpp"
#include "core/core.hpp"
#include "core/parser.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/player.hpp"
#include "world/room.hpp"
#include "world/time-weather.hpp"
#include "world/world.hpp"


// Constructor, sets up the parser.
Parser::Parser() : m_special_state(SpecialState::NONE)
{
    add_command("close <dir>", ParserCommand::CLOSE);
    add_command("drop <item:i>", ParserCommand::DROP);
    add_command("[equipment|equip|eq]", ParserCommand::EQUIPMENT);
    add_command("[equip|eq|wield|hold|wear] <item:i>", ParserCommand::EQUIP);
    add_command("exits", ParserCommand::EXITS);
    add_command("[fuck|shit|piss|bastard] *", ParserCommand::SWEAR);
    add_command("[go|travel|walk|run|move] <dir>", ParserCommand::GO);
    add_command("[inventory|invent|inv|i]", ParserCommand::INVENTORY);
    add_command("lock <dir>", ParserCommand::LOCK);
    add_command("[look|l]", ParserCommand::LOOK);
    add_command("no", ParserCommand::NO);
    add_command("[north|n|east|e|south|s|west|w|northeast|ne|northwest|nw|southeast|se|southwest|sw|up|u|down|d]", ParserCommand::DIRECTION);
    add_command("open <dir>", ParserCommand::OPEN);
    add_command("[quit|exit]", ParserCommand::QUIT);
    add_command("save", ParserCommand::SAVE);
    add_command("[take|get] <item:r>", ParserCommand::TAKE);
    add_command("[time|date]", ParserCommand::TIME);
    add_command("[unequip|uneq|remove] <item:e>", ParserCommand::UNEQUIP);
    add_command("unlock <dir>", ParserCommand::UNLOCK);
    add_command("wait", ParserCommand::WAIT);
    add_command("weather", ParserCommand::WEATHER);
    add_command("[xyzzy|frotz|plugh|plover]", ParserCommand::XYZZY);
    add_command("yes", ParserCommand::YES);
    add_command("#hash <txt>", ParserCommand::HASH);
    add_command("[#spawnitem|#si] <txt>", ParserCommand::SPAWN_ITEM);
    add_command("#tp <txt>", ParserCommand::TELEPORT);
}

// Adds a command to the parser.
void Parser::add_command(const std::string &text, ParserCommand cmd)
{
    if (!text.size()) throw std::runtime_error("Attempt to add empty parser command!");
    std::vector<std::string> words = StrX::string_explode(text, " ");
    if (!words.size()) throw std::runtime_error("Attempt to add empty parser command!");

    std::string first_word = words.at(0);
    words.erase(words.begin());
    
    // If the first word has multiple possible options, we'll add them all individually.
    if (first_word.size() > 2 && first_word.at(0) == '[' && first_word.at(first_word.size() - 1) == ']')
    {
        first_word = first_word.substr(1, first_word.size() - 2);
        const std::vector<std::string> word_exploded = StrX::string_explode(first_word, "|");
        for (auto word : word_exploded)
            add_command(word + (words.size() ? " " + StrX::collapse_vector(words) : ""), cmd);
        return;
    }

    // Add an individual command.
    ParserCommandData pcd;
    pcd.first_word = first_word;
    pcd.words = words;
    pcd.command = cmd;
    pcd.direction_match = pcd.target_match = pcd.any_length = false;
    for (auto word : words)
    {
        if (word.size() > 2 && word.at(0) == '<')
        {
            if (word.at(1) == 'd') pcd.direction_match = true;
            else pcd.target_match = true;
        }
        else if (word == "*") pcd.any_length = true;
    }
    m_commands.push_back(pcd);
}

// Parses input from the player!
void Parser::parse(std::string input)
{
    if (!input.size()) return;
    input = StrX::str_tolower(input);
    std::vector<std::string> words = StrX::string_explode(input, " ");
    if (!words.size()) return;

    const std::string first_word = words.at(0);
    words.erase(words.begin());

    for (auto pcd : m_commands)
    {
        if (pcd.first_word == first_word && (pcd.target_match || pcd.any_length || pcd.words.size() == words.size()))
        {
            parse_pcd(first_word, words, pcd);
            return;
        }
    }
    
    std::string msg = "{y}I'm sorry, I don't understand.";
    if (m_special_state == SpecialState::DISAMBIGUATION) msg += " If you wanted to {Y}clarify your choice{y}, please {Y}type the entire command{y}.";
    core()->message(msg);
    m_special_state = SpecialState::NONE;
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
        const std::shared_ptr<Item> item = inv->get(i);
        if (input.size() == 1 && input.at(0) == StrX::itos(item->parser_id(), 4)) return i; // If the hex ID matches, that's an easy one.

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

    // Determine the highest score.
    unsigned int highest_score = 0;
    for (unsigned int i = 0; i < inv_size; i++)
        if (item_scores.at(i) > highest_score) highest_score = item_scores.at(i);

    // No matches at all?
    if (!highest_score) return ItemMatch::NOT_FOUND;

    // Now loop again, see how many candidates we have.
    std::vector<uint32_t> candidates;
    for (unsigned int i = 0; i < inv_size; i++)
        if (item_scores.at(i) == highest_score) candidates.push_back(i);

    // If we just have one, great! That was easy!
    if (candidates.size() == 1) return candidates.at(0);

    // If not...
    std::string disambig = "{c}I'm not sure which one you mean! Did you mean ";
    std::vector<std::string> candidate_names;
    for (auto c : candidates)
        candidate_names.push_back("{C}" + inv->get(c)->name() + " {B}{" + StrX::itos(inv->get(c)->parser_id(), 4) + "}{c}");
    disambig += StrX::comma_list(candidate_names) + "?";
    core()->message(disambig);
    m_special_state = SpecialState::DISAMBIGUATION;

    return ItemMatch::UNCLEAR;
}

// Parses a known command.
void Parser::parse_pcd(const std::string &first_word, const std::vector<std::string> &words, ParserCommandData pcd)
{
    const std::shared_ptr<Mobile> player = core()->world()->player();
    Direction parsed_direction = Direction::NONE;
    uint32_t target = Target::NONE;
    
    // Check if a direction needs to be parsed.
    if (pcd.direction_match)
    {
        for (unsigned int i = 0; i < std::min(pcd.words.size(), words.size()); i++)
        {
            const std::string pcd_word = pcd.words.at(i);
            if (pcd_word == "<dir>")
                parsed_direction = parse_direction(words.at(i));
        }
    }

    // Check if a target needs to be parsed.
    if (pcd.target_match)
    {
        Target target_type = Target::NONE;
        for (unsigned int i = 0; i < std::min(pcd.words.size(), words.size()); i++)
        {
            const std::string pcd_word = pcd.words.at(i);
            if (pcd_word == "<item:i>") target_type = Target::ITEM_INV;
            else if (pcd_word == "<item:e>") target_type = Target::ITEM_EQU;
            else if (pcd_word == "<item:r>") target_type = Target::ITEM_ROOM;
            if (target_type == Target::NONE) continue;

            // Pick out the words used to match the target.
            std::vector<std::string> target_words(words.size() - i);
            std::copy(words.begin() + i, words.end(), target_words.begin());

            // Run the target-matching parser.
            std::shared_ptr<Inventory> inv_target = nullptr;
            switch (target_type)
            {
                case Target::ITEM_EQU: inv_target = player->equ(); break;
                case Target::ITEM_INV: inv_target = player->inv(); break;
                case Target::ITEM_ROOM: inv_target = core()->world()->get_room(player->location())->inv(); break;
                default: throw std::runtime_error("Unknown parser target type.");
            }
            target = parse_item_name(target_words, inv_target);
        }
    }

    const std::string collapsed_words = StrX::collapse_vector(words);
    switch (pcd.command)
    {
        case ParserCommand::NONE: break;
        case ParserCommand::DIRECTION:
            ActionTravel::travel(player, parse_direction(first_word));
            break;
        case ParserCommand::DROP:
            if (!words.size()) core()->message("{y}Please specify {Y}what you want to drop{y}.");
            else if (target == ItemMatch::NOT_FOUND) core()->message("{y}You don't seem to be carrying {Y}" + collapsed_words + "{y}.");
            else if (target <= ItemMatch::VALID) ActionInventory::drop(player, target);
            break;
        case ParserCommand::EQUIP:
            if (!words.size()) core()->message("{y}Please specify {Y}what you want to equip{y}.");
            else if (target == ItemMatch::NOT_FOUND) core()->message("{y}You don't seem to be carrying {Y}" + collapsed_words + "{y}.");
            else if (target <= ItemMatch::VALID) ActionInventory::equip(player, target);
            break;
        case ParserCommand::EQUIPMENT:
            ActionInventory::equipment(player);
            break;
        case ParserCommand::EXITS:
            ActionLook::obvious_exits(player, false);
            break;
        case ParserCommand::GO:
            if (parsed_direction == Direction::NONE) core()->message("{y}Please specify a {Y}direction {y}to travel.");
            else ActionTravel::travel(player, parsed_direction);
            break;
        case ParserCommand::HASH:
            if (!words.size()) core()->message("{y}Please specify a {Y}string to hash{y}.");
            else
            {
                const std::string hash_word = StrX::str_toupper(collapsed_words);
                core()->message("{G}" + hash_word + " {g}hashes to {G}" + std::to_string(StrX::hash(hash_word)) + "{g}.");
            }
            break;
        case ParserCommand::INVENTORY:
            ActionInventory::check_inventory(player);
            break;
        case ParserCommand::LOCK: case ParserCommand::UNLOCK:
            if (parsed_direction == Direction::NONE) core()->message("{y}Please specify a {Y}direction {y}to " + first_word + ".");
            else ActionDoors::lock_or_unlock(player, parsed_direction, pcd.command == ParserCommand::UNLOCK);
            break;
        case ParserCommand::LOOK: ActionLook::look(player); break;
        case ParserCommand::OPEN: case ParserCommand::CLOSE:
            if (parsed_direction == Direction::NONE) core()->message("{y}Please specify a {Y}direction {y}to " + first_word + ".");
            else ActionDoors::open_or_close(player, parsed_direction, pcd.command == ParserCommand::OPEN);
            break;
        case ParserCommand::QUIT:
            core()->message("{R}Are you sure you want to quit? {M}Your game will not be saved. {R}Type {C}yes {R}to confirm.");
            m_special_state = SpecialState::QUIT_CONFIRM;
            return; // not break
        case ParserCommand::SAVE:
            core()->save();
            break;
        case ParserCommand::SPAWN_ITEM:
            if (!words.size()) core()->message("{y}Please specify an {Y}item ID{y}.");
            else ActionsCheat::spawn_item(collapsed_words);
            break;
        case ParserCommand::SWEAR:
            core()->message("{y}Real adventurers do not use such language.");
            break;
        case ParserCommand::TAKE:
            if (!words.size()) core()->message("{y}Please specify {Y}what you want to take{y}.");
            else if (target == ItemMatch::NOT_FOUND) core()->message("{y}You don't see {Y}" + collapsed_words + "{y} here.");
            else if (target <= ItemMatch::VALID) ActionInventory::take(player, target);
            break;
        case ParserCommand::TELEPORT:
            if (!words.size()) core()->message("{y}Please specify a {Y}teleport destination{y}.");
            else ActionsCheat::teleport(collapsed_words);
            break;
        case ParserCommand::TIME:
            ActionLook::time(player);
            break;
        case ParserCommand::UNEQUIP:
            if (!words.size()) core()->message("{y}Please specify {Y}what you want to unequip{y}.");
            else if (target == ItemMatch::NOT_FOUND) core()->message("{y}You don't seem to be wearing or wielding {Y}" + collapsed_words + "{y}.");
            else if (target <= ItemMatch::VALID) ActionInventory::unequip(player, target);
            break;
        case ParserCommand::WAIT:
            core()->message("Time passes...");
            core()->world()->time_weather()->pass_time(TimeWeather::HOUR);
            break;
        case ParserCommand::WEATHER:
            ActionLook::weather(player);
            break;
        case ParserCommand::XYZZY:
            core()->message("{u}A hollow voice says, {m}\"Fool.\"");
            break;
        case ParserCommand::YES: case ParserCommand::NO:
            if (m_special_state == SpecialState::QUIT_CONFIRM)
            {
                if (pcd.command == ParserCommand::YES)
                {
                    core()->cleanup();
                    exit(EXIT_SUCCESS);
                }
                else core()->message("{y}Very well. Your adventure continues.");
            }
            else core()->message("{y}That was a rhetorical question.");
            break;
    }

    if (target != ItemMatch::UNCLEAR) m_special_state = SpecialState::NONE;
}
