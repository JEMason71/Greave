// core/parser.cpp -- The command parser! Converts player input into commands that the game can understand.
// Copyright (c) 2021 Raine "Gravecat" Simmons and the Greave contributors. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/cheat.hpp"
#include "actions/combat/melee.hpp"
#include "actions/doors.hpp"
#include "actions/help.hpp"
#include "actions/inventory.hpp"
#include "actions/look.hpp"
#include "actions/rest.hpp"
#include "actions/status.hpp"
#include "actions/travel.hpp"
#include "core/core.hpp"
#include "core/mathx.hpp"
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
    add_command("[attack|kill|k] <mobile>", ParserCommand::ATTACK);
    add_command("close <dir>", ParserCommand::CLOSE);
    add_command("drop <item:i>", ParserCommand::DROP);
    add_command("[equipment|equip|eq]", ParserCommand::EQUIPMENT);
    add_command("[equip|eq|wield|hold|wear] <item:i>", ParserCommand::EQUIP);
    add_command("[examine|exam|ex|x] <item:i|item:e|item:r|mobile>", ParserCommand::EXAMINE);
    add_command("exits", ParserCommand::EXITS);
    add_command("[fuck|shit|piss|bastard] *", ParserCommand::SWEAR);
    add_command("[go|travel|walk|run|move] <dir>", ParserCommand::GO);
    add_command("help *", ParserCommand::HELP);
    add_command("[inventory|invent|inv|i]", ParserCommand::INVENTORY);
    add_command("lock <dir>", ParserCommand::LOCK);
    add_command("[look|l]", ParserCommand::LOOK);
    add_command("[look|l] <item:i|item:e|item:r|mobile>", ParserCommand::EXAMINE);
    add_command("no", ParserCommand::NO);
    add_command("[north|n|east|e|south|s|west|w|northeast|ne|northwest|nw|southeast|se|southwest|sw|up|u|down|d]", ParserCommand::DIRECTION);
    add_command("open <dir>", ParserCommand::OPEN);
    add_command("[quit|exit]", ParserCommand::QUIT);
    add_command("save", ParserCommand::SAVE);
    add_command("[sa|sb|sd]", ParserCommand::STANCE);
    add_command("[score|sc]", ParserCommand::SCORE);
    add_command("[skills|skill|sk]", ParserCommand::SKILLS);
    add_command("stance <txt>", ParserCommand::STANCE);
    add_command("[take|get] <item:r>", ParserCommand::TAKE);
    add_command("[time|date]", ParserCommand::TIME);
    add_command("[unequip|uneq|remove] <item:e>", ParserCommand::UNEQUIP);
    add_command("unlock <dir>", ParserCommand::UNLOCK);
    add_command("[wait|rest|sleep|zzz|z] <txt>", ParserCommand::WAIT);
    add_command("[weather|temperature|temp]", ParserCommand::WEATHER);
    add_command("[xyzzy|frotz|plugh|plover]", ParserCommand::XYZZY);
    add_command("yes", ParserCommand::YES);
    add_command("#bix <txt>", ParserCommand::MIXUP_BIG);
    add_command("#hash <txt>", ParserCommand::HASH);
    add_command("#mix <txt>", ParserCommand::MIXUP);
    add_command("#money <txt>", ParserCommand::ADD_MONEY);
    add_command("[#spawnitem|#si] <txt>", ParserCommand::SPAWN_ITEM);
    add_command("[#spawnmobile|#spawnmob|#sm] <txt>", ParserCommand::SPAWN_MOBILE);
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

// Tells the player how to confirm a command.
void Parser::confirm_message()
{ core()->message("{0}{m}If you are sure you want to do this, repeat your command with a {M}! {m}at the beginning (for example, {M}!" + m_last_input + "{m})."); }

// Parses input from the player!
void Parser::parse(std::string input)
{
    if (!input.size()) return;
    m_last_input = input;
    input = StrX::str_tolower(input);
    std::vector<std::string> words = StrX::string_explode(input, " ");
    if (!words.size()) return;

    std::string first_word = words.at(0);
    words.erase(words.begin());
    bool confirm_command = false;
    if (first_word.size() > 1 && first_word.at(0) == '!')
    {
        first_word = first_word.substr(1);
        confirm_command = true;
    }

    for (auto pcd : m_commands)
    {
        if (pcd.first_word == first_word && (pcd.target_match || pcd.any_length || pcd.words.size() == words.size()))
        {
            parse_pcd(first_word, words, pcd, confirm_command);
            return;
        }
    }

    std::string msg = "{y}I'm sorry, I don't understand. Type {Y}HELP {y}for help.";
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

// Wrapper function to check for out of range values.
int32_t Parser::parse_int(const std::string &s)
{
    try
    {
        return (std::stoll(s));
    }
    catch(const std::exception& e)
    {
        return INT_MAX;
    }
}

// Attempts to match a name to a given target.
Parser::ParserSearchResult Parser::parse_target(std::vector<std::string> input, ParserTarget target)
{
    if (!input.size()) return { 0, "", "", 0, 0, ParserTarget::TARGET_NONE, 0 };   // Don't even *try* with blank inputs.

    std::vector<ParserSearchResult> candidates;
    const std::shared_ptr<World> world = core()->world();
    const std::shared_ptr<Player> player = world->player();
    const uint32_t player_location = player->location();
    int count = -1;

    // First, check if the player has specified a number (not the numerical ID of an item or mobile, but a count like dropping 10 arrows).
    if (input.size() >= 2 && StrX::is_number(input.at(0)))
    {
        count = parse_int(input.at(0));
        input.erase(input.begin());
    }

    // Items the player has equipped.
    const std::shared_ptr<Inventory> equ = player->equ();
    if ((target & ParserTarget::TARGET_EQUIPMENT) == ParserTarget::TARGET_EQUIPMENT)
    {
        for (uint32_t i = 0; i < equ->count(); i++)
        {
            const std::shared_ptr<Item> item = equ->get(i);
            candidates.push_back({0, StrX::str_tolower(item->name(Item::NAME_FLAG_NO_COLOUR)), StrX::str_tolower(item->name(Item::NAME_FLAG_NO_COLOUR | Item::NAME_FLAG_NO_COUNT)),
                item->parser_id(), i, ParserTarget::TARGET_EQUIPMENT, count});
        }
    }

    // Items the player is carrying.
    const std::shared_ptr<Inventory> inv = player->inv();
    if ((target & ParserTarget::TARGET_INVENTORY) == ParserTarget::TARGET_INVENTORY)
    {
        for (uint32_t i = 0; i < inv->count(); i++)
        {
            const std::shared_ptr<Item> item = inv->get(i);
            candidates.push_back({0, StrX::str_tolower(item->name(Item::NAME_FLAG_NO_COLOUR)), StrX::str_tolower(item->name(Item::NAME_FLAG_NO_COLOUR | Item::NAME_FLAG_NO_COUNT)),
                item->parser_id(), i, ParserTarget::TARGET_INVENTORY, count});
        }
    }

    // Items in the room the player is at.
    const std::shared_ptr<Inventory> room_inv = world->get_room(player_location)->inv();
    if ((target & ParserTarget::TARGET_ROOM) == ParserTarget::TARGET_ROOM)
    {
        for (uint32_t i = 0; i < room_inv->count(); i++)
        {
            const std::shared_ptr<Item> item = room_inv->get(i);
            candidates.push_back({0, StrX::str_tolower(item->name(Item::NAME_FLAG_NO_COLOUR)), StrX::str_tolower(item->name(Item::NAME_FLAG_NO_COLOUR | Item::NAME_FLAG_NO_COUNT)),
                item->parser_id(), i, ParserTarget::TARGET_ROOM, count});
        }
    }

    // Mobiles in the player's room.
    for (uint32_t i = 0; i < world->mob_count(); i++)
    {
        const std::shared_ptr<Mobile> mob = world->mob_vec(i);
        if (mob->location() == player_location) candidates.push_back({0, StrX::str_tolower(mob->name(Item::NAME_FLAG_NO_COLOUR)), "", mob->parser_id(), i,
            ParserTarget::TARGET_MOBILE, count});
    }

    // Score each candidate.
    for (uint32_t i = 0; i < candidates.size(); i++)
    {
        // If the parser ID matches the player's input, that's an easy one.
        if (input.size() == 1 && input.at(0) == StrX::itos(candidates.at(i).parser_id, 4))
        {
            candidates.at(i).score = 1000;
            return candidates.at(i);
        }

        // Score each target by matching words in its name.
        std::vector<std::string> name_words = StrX::string_explode(candidates.at(i).name, " ");
        const std::string collapsed_input = StrX::collapse_vector(input);
        if (candidates.at(i).name == collapsed_input || candidates.at(i).name_np == collapsed_input) candidates.at(i).score = 100;
        else
        {
            int score = 0, singular_score = 0;
            const bool calc_singular_score = (candidates.at(i).name_np.size() && candidates.at(i).name_np != candidates.at(i).name);
            bool invalid_word = false;
            for (size_t j = 0; j < input.size(); j++)
            {
                bool word_in_name = false;
                for (size_t k = 0; k < name_words.size(); k++)
                {
                    if (input.at(j) == name_words.at(k))
                    {
                        score++;
                        word_in_name = true;
                    }
                }
                if (!word_in_name) invalid_word = true;
            }
            if (invalid_word) score = 0;
            if (calc_singular_score)
            {
                invalid_word = false;
                std::vector<std::string> name_words_singular = StrX::string_explode(candidates.at(i).name_np, " ");
                for (size_t j = 0; j < input.size(); j++)
                {
                    bool word_in_name = false;
                    for (size_t k = 0; k < name_words_singular.size(); k++)
                    {
                        if (input.at(j) == name_words_singular.at(k))
                        {
                            singular_score++;
                            word_in_name = true;
                        }
                    }
                    if (!word_in_name) invalid_word = true;
                }
                if (invalid_word) singular_score = 0;
            }
            candidates.at(i).score = std::max(score, singular_score);
        }
    }

    // Determine the highest score.
    int highest_score = 0;
    for (size_t i = 0; i < candidates.size(); i++)
        if (candidates.at(i).score > highest_score) highest_score = candidates.at(i).score;

    // No matches at all?
    if (!highest_score) return { 0, "", "", 0, 0, ParserTarget::TARGET_NONE, 0 };

    // Now strip out any candidates that don't match the highest score.
    for (size_t i = 0; i < candidates.size(); i++)
    {
        if (candidates.at(i).score < highest_score)
        {
            candidates.erase(candidates.begin() + i);
            i--;
        }
    }

    // If we just have one, great! That was easy!
    if (candidates.size() == 1) return candidates.at(0);

    // If not...
    std::string disambig = "{c}I'm not sure which one you mean! Did you mean ";
    std::vector<std::string> candidate_names;
    for (auto c : candidates)
        candidate_names.push_back("{C}" + c.name + " {B}{" + StrX::itos(c.parser_id, 4) + "}{c}");
    disambig += StrX::comma_list(candidate_names) + "?";
    core()->message(disambig);
    m_special_state = SpecialState::DISAMBIGUATION;
    return { 0, "", "", 0, 0, ParserTarget::TARGET_UNCLEAR, 0 };
}

// Parses a known command.
void Parser::parse_pcd(const std::string &first_word, const std::vector<std::string> &words, ParserCommandData pcd, bool confirm)
{
    const std::shared_ptr<Mobile> player = core()->world()->player();
    Direction parsed_direction = Direction::NONE;
    uint32_t parsed_target = 0;
    ParserTarget parsed_target_type = ParserTarget::TARGET_NONE;
    int parsed_target_count = -1;

    // Check if a direction needs to be parsed.
    if (pcd.direction_match)
    {
        for (size_t i = 0; i < std::min(pcd.words.size(), words.size()); i++)
        {
            const std::string pcd_word = pcd.words.at(i);
            if (pcd_word == "<dir>")
                parsed_direction = parse_direction(words.at(i));
        }
    }

    // Check if a target needs to be parsed.
    if (pcd.target_match)
    {
        for (size_t i = 0; i < pcd.words.size(); i++)
        {
            uint32_t target_flags = 0;
            const std::string pcd_word = pcd.words.at(i);
            if (pcd_word.find("item:i") != std::string::npos) target_flags ^= ParserTarget::TARGET_INVENTORY;
            if (pcd_word.find("item:e") != std::string::npos) target_flags ^= ParserTarget::TARGET_EQUIPMENT;
            if (pcd_word.find("item:r") != std::string::npos) target_flags ^= ParserTarget::TARGET_ROOM;
            if (pcd_word.find("mobile") != std::string::npos) target_flags ^= ParserTarget::TARGET_MOBILE;
            if (!target_flags) continue;

            if (words.size() <= i)
            {
                // Normally we'd just skip trying to parse this if the player hasn't given enough input.
                // BUT...! For target-only matches, there's a special exception. This allows the player
                // to do something like "kill goblin", then just type "kill" again to keep attacking
                // the same target without naming it over and over.
                if (target_flags == ParserTarget::TARGET_MOBILE)
                {
                    // Check if the player has a *valid* Mobile targetted already. mob_target() will
                    // return 0 if there is no target, no valid target, or the valid target is no
                    // longer in the same room, which saves us some effort here.
                    uint32_t target_id = core()->world()->player()->mob_target();
                    if (!target_id) continue;

                    for (size_t i = 0; i < core()->world()->mob_count(); i++)
                    {
                        const auto mob = core()->world()->mob_vec(i);
                        if (mob->id() == target_id)
                        {
                            parsed_target = i;
                            parsed_target_type = ParserTarget::TARGET_MOBILE;
                            core()->message("{0}{m}(" + mob->name(Mobile::NAME_FLAG_THE | Mobile::NAME_FLAG_NO_COLOUR) + ")");
                            break;
                        }
                    }
                }
                continue;
            }

            // Pick out the words used to match the target.
            std::vector<std::string> target_words(words.size() - i);
            std::copy(words.begin() + i, words.end(), target_words.begin());

            // Run the target-matching parser.
            ParserSearchResult psr = parse_target(target_words, static_cast<ParserTarget>(target_flags));
            parsed_target = psr.target;
            parsed_target_type = psr.type;
            parsed_target_count = psr.count;

            // Auto-target matching only applies to targets that are mobile-only:
            if (target_flags == ParserTarget::TARGET_MOBILE)
            {
                // Clear any existing auto-target, if the player enters something uncertain.
                if (psr.type == ParserTarget::TARGET_NONE || psr.type == ParserTarget::TARGET_UNCLEAR)
                    core()->world()->player()->set_mob_target(0);

                // If a target was picked, update the player's auto-target.
                else if (psr.type == ParserTarget::TARGET_MOBILE)
                    core()->world()->player()->set_mob_target(core()->world()->mob_vec(psr.target)->id());
            }
        }
    }

    const std::string collapsed_words = StrX::collapse_vector(words);
    switch (pcd.command)
    {
        case ParserCommand::NONE: break;
        case ParserCommand::ADD_MONEY:
            if (!words.size() || !StrX::is_number(words.at(0))) core()->message("{y}Please specify {Y}how many coins to add{y}.");
            else ActionCheat::add_money(parse_int(words.at(0)));
            break;
        case ParserCommand::ATTACK:
            if (parsed_target_type == ParserTarget::TARGET_MOBILE) Melee::attack(player, core()->world()->mob_vec(parsed_target));
            else if (!words.size()) core()->message("{y}Please specify {Y}what you want to attack{y}.");
            else if (parsed_target_type == ParserTarget::TARGET_NONE) core()->message("{y}You don't see any such {Y}" + collapsed_words + " {y}here.");
            break;
        case ParserCommand::DIRECTION:
            ActionTravel::travel(player, parse_direction(first_word), confirm);
            break;
        case ParserCommand::DROP:
            if (!words.size()) core()->message("{y}Please specify {Y}what you want to drop{y}.");
            else if (parsed_target_type == ParserTarget::TARGET_NONE) core()->message("{y}You don't seem to be carrying {Y}" + collapsed_words + "{y}.");
            else if (parsed_target_type == ParserTarget::TARGET_INVENTORY) ActionInventory::drop(player, parsed_target, parsed_target_count);
            break;
        case ParserCommand::EQUIP:
            if (!words.size()) core()->message("{y}Please specify {Y}what you want to equip{y}.");
            else if (parsed_target_type == ParserTarget::TARGET_NONE) core()->message("{y}You don't seem to be carrying {Y}" + collapsed_words + "{y}.");
            else if (parsed_target_type == ParserTarget::TARGET_INVENTORY) ActionInventory::equip(player, parsed_target);
            break;
        case ParserCommand::EQUIPMENT:
            ActionInventory::equipment();
            break;
        case ParserCommand::EXAMINE:
            if (!words.size()) core()->message("{y}Please specify {Y}what you want to examine{y}.");
            else if (parsed_target_type == ParserTarget::TARGET_NONE) core()->message("{y}You don't see any such {Y}" + collapsed_words + "{y} here.");
            else if (parsed_target_type != ParserTarget::TARGET_UNCLEAR) ActionLook::examine(parsed_target_type, parsed_target);
            break;
        case ParserCommand::EXITS:
            ActionLook::obvious_exits(false);
            break;
        case ParserCommand::GO:
            if (parsed_direction == Direction::NONE) core()->message("{y}Please specify a {Y}direction {y}to travel.");
            else ActionTravel::travel(player, parsed_direction, confirm);
            break;
        case ParserCommand::HASH:
            if (!words.size()) core()->message("{y}Please specify a {Y}string to hash{y}.");
            else
            {
                const std::string hash_word = StrX::str_toupper(collapsed_words);
                core()->message("{G}" + hash_word + " {g}hashes to {G}" + std::to_string(StrX::hash(hash_word)) + "{g}.");
            }
            break;
        case ParserCommand::HELP:
            ActionHelp::help(StrX::collapse_vector(words));
            break;
        case ParserCommand::INVENTORY:
            ActionInventory::check_inventory();
            break;
        case ParserCommand::LOCK: case ParserCommand::UNLOCK:
            if (parsed_direction == Direction::NONE) core()->message("{y}Please specify a {Y}direction {y}to " + first_word + ".");
            else ActionDoors::lock_or_unlock(player, parsed_direction, pcd.command == ParserCommand::UNLOCK);
            break;
        case ParserCommand::LOOK: ActionLook::look(); break;
        case ParserCommand::MIXUP:
        case ParserCommand::MIXUP_BIG:
            if (!words.size() || !StrX::is_number(words.at(0))) core()->message("{y}Please specify a {Y}number to mix up{y}.");
            else
            {
                const bool big_mix = (pcd.command == ParserCommand::MIXUP_BIG);
                core()->message("{G}" + words.at(0) + " {g}mixes to {G}" + std::to_string(MathX::mixup(parse_int(words.at(0)), (big_mix ? 2 : 10))) + "{g}.");
                break;
            }
            break;
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
        case ParserCommand::SCORE:
            ActionStatus::score();
            break;
        case ParserCommand::SKILLS:
            ActionStatus::skills();
            break;
        case ParserCommand::SPAWN_ITEM:
            if (!words.size()) core()->message("{y}Please specify an {Y}item ID{y}.");
            else ActionCheat::spawn_item(collapsed_words);
            break;
        case ParserCommand::SPAWN_MOBILE:
            if (!words.size()) core()->message("{y}Please specify a {Y}mobile ID{Y}.");
            else ActionCheat::spawn_mobile(collapsed_words);
            break;
        case ParserCommand::STANCE:
        {
            CombatStance chosen_stance = static_cast<CombatStance>(0xFF);
            if (!words.size() || !words.at(0).size())
            {
                if (first_word.size() == 2)
                {
                    switch (first_word[1])
                    {
                        case 'a': chosen_stance = CombatStance::AGGRESSIVE; break;
                        case 'b': chosen_stance = CombatStance::BALANCED; break;
                        case 'd': chosen_stance = CombatStance::DEFENSIVE; break;
                    }
                }
            }
            else switch (words.at(0)[0])
            {
                case 'a': chosen_stance = CombatStance::AGGRESSIVE; break;
                case 'b': chosen_stance = CombatStance::BALANCED; break;
                case 'd': chosen_stance = CombatStance::DEFENSIVE; break;
            }
            if (chosen_stance == static_cast<CombatStance>(0xFF)) core()->message("{y}Please choose a stance ({Y}aggressive{y}, {Y}defensive{y} or {Y}balanced{y}).");
            else Combat::change_stance(player, chosen_stance);
            break;
        }
        case ParserCommand::SWEAR:
            core()->message("{y}Real adventurers do not use such language.");
            break;
        case ParserCommand::TAKE:
            if (!words.size()) core()->message("{y}Please specify {Y}what you want to take{y}.");
            else if (parsed_target_type == ParserTarget::TARGET_NONE) core()->message("{y}You don't see {Y}" + collapsed_words + "{y} here.");
            else if (parsed_target_type == ParserTarget::TARGET_ROOM) ActionInventory::take(player, parsed_target, parsed_target_count);
            break;
        case ParserCommand::TELEPORT:
            if (!words.size()) core()->message("{y}Please specify a {Y}teleport destination{y}.");
            else ActionCheat::teleport(collapsed_words);
            break;
        case ParserCommand::TIME:
            ActionStatus::time();
            break;
        case ParserCommand::UNEQUIP:
            if (!words.size()) core()->message("{y}Please specify {Y}what you want to unequip{y}.");
            else if (parsed_target_type == ParserTarget::TARGET_NONE) core()->message("{y}You don't seem to be wearing or wielding {Y}" + collapsed_words + "{y}.");
            else if (parsed_target_type == ParserTarget::TARGET_EQUIPMENT) ActionInventory::unequip(player, parsed_target);
            break;
        case ParserCommand::WAIT:
            ActionRest::rest(first_word, words);
            break;
        case ParserCommand::WEATHER:
            ActionStatus::weather();
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

    if (parsed_target_type != ParserTarget::TARGET_UNCLEAR) m_special_state = SpecialState::NONE;
}
