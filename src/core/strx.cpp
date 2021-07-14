// core/strx.cpp -- Various utility functions that deal with string manipulation/conversion.
// Copyright (c) 2009-2021 Raine "Gravecat" Simmons and the Greave contributors. Licensed under the GNU Affero General Public License v3 or any later version.

#include "core/core.hpp"
#include "core/guru.hpp"
#include "core/mathx.hpp"
#include "core/strx.hpp"
#include "world/room.hpp"

#include <algorithm>
#include <iterator>
#include <regex>
#include <sstream>


const int StrX::CL_FLAG_USE_AND = 1, StrX::CL_FLAG_OXFORD_COMMA = 2, StrX::CL_FLAG_NO_OR = 4;   // comma_list() flags


// Capitalizes the first letter of a string.
std::string StrX::capitalize_first_letter(std::string str)
{
    if (str.size() && str[0] >= 'a' && str[0] <= 'z') str[0] -= 32;
    return str;
}

// Simple function to collapse a string vector into words.
std::string StrX::collapse_vector(std::vector<std::string> vec)
{
    std::ostringstream output;
    if (!vec.empty())
    {
        std::copy(vec.begin(), vec.end() - 1, std::ostream_iterator<std::string>(output, " "));
        output << vec.back();
    }
    return output.str();
}

// As above, but for an integer vector>
std::string StrX::collapse_vector(std::vector<uint32_t> vec)
{
    std::vector<std::string> converted_vec;
    for (auto i : vec)
        converted_vec.push_back(std::to_string(i));
    return collapse_vector(converted_vec);
}

std::string StrX::comma_list(std::vector<std::string> vec, int flags)
{
    const bool use_and = ((flags & CL_FLAG_USE_AND) == CL_FLAG_USE_AND);
    const bool oxford_comma = ((flags & CL_FLAG_OXFORD_COMMA) == CL_FLAG_OXFORD_COMMA);
    const bool no_or = ((flags & CL_FLAG_NO_OR) == CL_FLAG_NO_OR);
    if (!vec.size())
    {
        core()->guru()->nonfatal("Empty vector provided to comma_list!", Guru::WARN);
        return "";
    }
    if (vec.size() == 1) return vec.at(0);
    std::string plus = " and ";
    if (!use_and)
    {
        if (no_or) plus = " ";
        else plus = " or ";
    }
    if (oxford_comma) plus = "," + plus;
    else if (vec.size() == 2) return vec.at(0) + plus + vec.at(1);

    std::string str;
    for (size_t i = 0; i < vec.size(); i++)
    {
        str += vec.at(i);
        if (i < vec.size() - 1)
        {
            if (i == vec.size() - 2) str += plus;
            else str += ", ";
        }
    }

    return str;
}

// Counts all the colour tags in a string.
size_t StrX::count_colour_tags(const std::string &str)
{
    size_t tags = 0;
    for (size_t i = 0; i < str.size(); i++)
        if (str.at(i) == '{' && str.size() > i + 2 && str.at(i + 2) == '}') tags++;
    return tags;
}

// Decodes a compressed string (e.g. 4cab2z becomes ccccabzz).
std::string StrX::decode_compressed_string(std::string cb)
{
    std::string result;
    while(cb.size())
    {
        std::string letter = cb.substr(0, 1);
        cb = cb.substr(1);
        if (letter[0] >= '0' && letter[0] <= '9')
        {
            int number = letter[0] - '0';
            letter = cb.substr(0, 1);
            cb = cb.substr(1);
            while (letter[0] >= '0' && letter[0] <= '9')
            {
                number *= 10;
                number += letter[0] - '0';
                letter = cb.substr(0, 1);
                cb = cb.substr(1);
            }
            result += std::string(number, letter[0]);
        }
        else result += letter;
    }
    return result;
}

// Converts a direction enum into a string.
std::string StrX::dir_to_name(Direction dir, DirNameType dnt)
{
    std::string prefix;
    if (dnt == DirNameType::TO_THE || dnt == DirNameType::TO_THE_ALT || dnt == DirNameType::FROM_THE || dnt == DirNameType::FROM_THE_ALT)
    {
        if (dir == Direction::UP) return (dnt == DirNameType::TO_THE || dnt == DirNameType::FROM_THE ? "above" : "up");
        else if (dir == Direction::DOWN) return (dnt == DirNameType::TO_THE || dnt == DirNameType::FROM_THE ? "below" : "down");
        else prefix = (dnt == DirNameType::TO_THE || dnt == DirNameType::TO_THE_ALT ? "to the " : "from the ");
    }

    switch(dir)
    {
        case Direction::NORTH: return prefix + "north";
        case Direction::SOUTH: return prefix + "south";
        case Direction::EAST: return prefix + "east";
        case Direction::WEST: return prefix + "west";
        case Direction::NORTHEAST: return prefix + "northeast";
        case Direction::NORTHWEST: return prefix + "northwest";
        case Direction::SOUTHEAST: return prefix + "southeast";
        case Direction::SOUTHWEST: return prefix + "southwest";
        case Direction::UP: return prefix + "up";
        case Direction::DOWN: return prefix + "down";
        case Direction::NONE: return "????";
        default:
            throw std::runtime_error("Invalid direction enum: " + std::to_string(static_cast<int>(dir)));
            return "";
    }
}

// As above, but with an integer instead of an enum.
std::string StrX::dir_to_name(uint8_t dir, DirNameType dnt) { return dir_to_name(static_cast<Direction>(dir), dnt); }

// Find and replace one string with another.
bool StrX::find_and_replace(std::string &input, const std::string &to_find, const std::string &to_replace)
{
    std::string::size_type pos = 0;
    const std::string::size_type find_len = to_find.length(), replace_len = to_replace.length();
    if (find_len == 0) return false;
    bool found = false;
    while ((pos = input.find(to_find, pos)) != std::string::npos)
    {
        found = true;
        input.replace(pos, find_len, to_replace);
        pos += replace_len;
    }
    return found;
}

// Converts a float or double to a string.
std::string StrX::ftos(double num, bool force_decimal)
{
    std::stringstream ss;
    ss << num;
    return ss.str() + (force_decimal && (static_cast<int>(num) == num) ? ".0" : "");
}

// FNV string hash function.
uint32_t StrX::hash(const std::string &str)
{
    uint32_t result = 2166136261U;
    std::string::const_iterator end = str.end();
    for (std::string::const_iterator iter = str.begin(); iter != end; ++iter)
        result = 127 * result + static_cast<unsigned char>(*iter);
    return result;
}

// Converts a hex string back to an integer.
uint32_t StrX::htoi(const std::string &hex_str)
{
    std::stringstream ss;
    ss << std::hex << hex_str;
    uint32_t result;
    ss >> result;
    return result;
}

// Returns a 'pretty' version of a number in string format, such as "12,345".
std::string StrX::intostr_pretty(int num)
{
    bool negative = false;
    if (num < 0)
    {
        negative = true;
        num = 0 - num;
    }
    std::string str = std::to_string(num), output;

    // If the number is 3 or less characters long, there's no need for any processing.
    if (str.length() <= 3) return((negative ? "-" : "") + str);

    do
    {
        // Cut the string up, and insert commas where appropriate.
        output = str.substr(str.length() - 3, 3) + "," + output;
        str = str.substr(0, str.length() - 3);
    } while (str.length() > 3);

    // Combine the results.
    std::string result = str + "," + output;

    // Remove the trailing comma.
    result = result.substr(0, result.length() - 1);

    return((negative ? "-" : "") + result);
}

// Checks if a string is a number.
bool StrX::is_number(const std::string &str)
{
    if (str.empty()) return false;
    size_t begin = 0;
    if (str[0] == '-') begin = 1;
    return std::all_of(str.begin() + begin, str.end(), ::isdigit);
}

// Checks if a character is a vowel.
bool StrX::is_vowel(char ch)
{
    if (ch >= 'A' && ch <= 'Z') ch += 32;
    return (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u');
}

// Converts an integer into a hex string.
std::string StrX::itoh(uint32_t num, size_t min_len)
{
    std::stringstream ss;
    ss << std::hex << num;
    std::string hex = ss.str();
    while (min_len && hex.size() < min_len) hex = "0" + hex;
    return hex;
}

// Converts an integer to a string, but optionally pads it to a minimum length with leading zeroes.
std::string StrX::itos(uint32_t num, size_t min_len)
{
    std::string result = std::to_string(num);
    while (result.size() < min_len) result = "0" + result;
    return result;
}

// Converts a metadata map into a string.
std::string StrX::metadata_to_string(const std::map<std::string, std::string> &metadata)
{
    std::string output;
    if (metadata.size())
    {
        for (auto mde : metadata)
            output += mde.first + ":" + mde.second + " ";
        output.pop_back();
    }
    return output;
}

// Converts a coin value into a mithril/gold/silver/copper ANSI string.
std::string StrX::mgsc_string(uint32_t coin, StrX::MGSC mode)
{
    const uint32_t mithril = coin / 1000000;
    const uint32_t gold = (coin - (mithril * 1000000)) / 1000;
    const uint32_t silver = (coin - (mithril * 1000000) - (gold * 1000)) / 10;
    const uint32_t copper = (coin - (mithril * 1000000) - (gold * 1000) - (silver * 10));
    const uint32_t total_coins = mithril + gold + silver + copper;
    if (mode == MGSC::SHORT || mode == MGSC::SHORT_ROUND)
    {
        std::string mithril_string, gold_string, silver_string, copper_string;
        if (mithril) mithril_string = "{C}" + intostr_pretty(mithril) + "m";
        if (gold) gold_string = "{Y}" + std::to_string(gold) + "g";
        if (silver) silver_string = "{w}" + std::to_string(silver) + "s";
        if (copper) copper_string = "{y}" + std::to_string(copper) + "c";
        if (mode == MGSC::SHORT_ROUND)
        {
            if (mithril >= 100) gold_string = "";
            if (mithril) silver_string = "";
            if (gold >= 100 || mithril) copper_string = "";
        }
        return mithril_string + gold_string + silver_string + copper_string;
    }
    else
    {
        std::vector<std::string> result_vec;
        if (mithril) result_vec.push_back(intostr_pretty(mithril) + " mithril");
        if (gold) result_vec.push_back(std::to_string(gold) + " gold");
        if (silver) result_vec.push_back(std::to_string(silver) + " silver");
        if (copper) result_vec.push_back(std::to_string(copper) + " copper");
        if (result_vec.size()) return comma_list(result_vec, CL_FLAG_USE_AND) + (mode == MGSC::LONG_COINS ? (total_coins  == 1 ? " {w}coin" : " {w}coins") : "");
        else return "zero";
    }
}

// Converts small numbers into words.
// Thanks to Josh Homann on StackOverflow for this one: https://stackoverflow.com/questions/40252753/c-converting-number-to-words
std::string StrX::number_to_word(uint64_t number)
{
    static const std::vector<std::string> ones { "", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine" };
    static const std::vector<std::string> teens { "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen","sixteen", "seventeen", "eighteen", "nineteen" };
    static const std::vector<std::string> tens { "", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety" };

    if (number < 10) return ones[number];
    else if (number < 20) return teens[number - 10];
    else if (number < 100) return tens[number / 10] + ((number % 10 != 0) ? "-" + number_to_word(number % 10) : "");
    else if (number < 1000) return number_to_word(number / 100) + " hundred" + ((number % 100 != 0) ? " " + number_to_word(number % 100) : "");
    else if (number < 1000000) return number_to_word(number / 1000) + " thousand" + ((number % 1000 != 0) ? " " + number_to_word(number % 1000) : "");
    else if (number < 1000000000UL) return number_to_word(number / 1000000) + " million" + ((number % 1000000 != 0) ? " " + number_to_word(number % 1000000) : "");
    else if (number < 1000000000000ULL) return number_to_word(number / 1000000000UL) + " billion" + ((number % 1000000000UL != 0) ? " " + number_to_word(number % 1000000000UL) : "");
    else return intostr_pretty(number);
}

// Makes a string into a possessive noun (e.g. orc = orc's, platypus = platypus')
std::string StrX::possessive_string(const std::string &str)
{
    if (!str.size()) return "";
    if (str[str.size() - 1] == 's' || str[str.size() - 1] == 'S') return str + "'";
    else return str + "'s";
}

// Makes pretty rainbow text!
std::string StrX::rainbow_text(const std::string &str, const std::string &colours)
{
    std::string output;
    int position = 0;
    int direction = 1;

    for (auto letter : str)
    {
        output += "{" + std::string(1, colours[position]) + "}" + std::string(1, letter);
        position += direction;
        if (position >= static_cast<int>(colours.size()))
        {
            position -= 2;
            direction = -1;
        }
        else if (position < 0)
        {
            position = 1;
            direction = 1;
        }
    }

    return output;
}

// Calls MathX::round_to_two(), then returns the result as a string.
std::string StrX::round_to_two(double num) { return ftos(MathX::round_to_two(num)); }

// Converts a std::string vector into a uint32_t vector.
std::vector<uint32_t> StrX::stoi_vec(std::vector<std::string> vec)
{
    std::vector<uint32_t> output;
    for (auto str : vec)
        output.push_back(std::stoul(str));
    return output;
}

// Converts a string to lower-case.
std::string StrX::str_tolower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

// Converts a string to upper-case.
std::string StrX::str_toupper(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

// String split/explode function.
std::vector<std::string> StrX::string_explode(std::string str, const std::string &separator)
{
    std::vector<std::string> results;

    std::string::size_type pos = str.find(separator, 0);
    const size_t pit = separator.length();

    while(pos != std::string::npos)
    {
        if (pos == 0) results.push_back("");
        else results.push_back(str.substr(0, pos));
        str.erase(0, pos + pit);
        pos = str.find(separator, 0);
    }
    results.push_back(str);

    return results;
}

// Similar to string_explode(), but takes colour and high/low-ASCII tags into account, and wraps to a given line length.
std::vector<std::string> StrX::string_explode_colour(const std::string &str, size_t line_len)
{
    std::vector<std::string> output;

    // Check to see if the line of text has the no-split tag at the start.
    if (str.size() >= 3)
    {
        if (!str.substr(0, 3).compare("{_}"))
        {
            output.push_back(str.substr(3));
            return output;
        }
    }

    // Check to see if the line is too short to be worth splitting.
    if (strlen_colour(str) <= line_len && str.find("{nl}") != std::string::npos && str.find("{lb}") != std::string::npos)
    {
        output.push_back(str);
        return output;
    }

    // Split the string into individual words.
    std::vector<std::string> words = string_explode(str, " ");

    // Keep track of the current line and our position on it.
    unsigned int current_line = 0, line_pos = 0;
    std::string last_colour = "{w}";    // The last colour tag we encountered; white by default.

    // Start with an empty string.
    output.push_back("");

    for (auto word : words)
    {
        if (word == "{nl}") // Check for new-line marker.
        {
            if (line_pos > 0)
            {
                line_pos = 0;
                current_line += 2;
                output.push_back(" ");
                output.push_back(last_colour);  // Start the line with the last colour tag we saw.
            }
        }
        else if (word == "{lb}")
        {
            if (line_pos > 0)
            {
                line_pos = 0;
                current_line += 1;
                output.push_back(last_colour);  // Start the line with the last colour tag we saw.
            }
        }
        else
        {
            size_t length = word.length();    // Find the length of the word.

            const size_t colour_count = count_colour_tags(word);    // Count the colour tags.
            if (colour_count) length -= (colour_count * 3);         // Reduce the length if one or more colour tags are found.
            if (length + line_pos >= line_len)  // Is the word too long for the current line?
            {
                line_pos = 0; current_line++;   // CR;LF
                output.push_back(last_colour);  // Start the line with the last colour tag we saw.
            }
            if (colour_count)
            {
                // Duplicate the last-used colour tag.
                const std::string::size_type flo = word.find_last_of("{");
                if (flo != std::string::npos && word.size() >= flo + 3) last_colour = word.substr(flo, 3);
            }
            if (line_pos != 0)  // NOT the start of a new line?
            {
                length++;
                output.at(current_line) += " ";
            }

            // Is the word STILL too long to fit over a single line?
            while (length > line_len)
            {
                const std::string trunc = word.substr(0, line_len);
                word = word.substr(line_len);
                output.at(current_line) += trunc;
                line_pos = 0;
                current_line++;
                output.push_back(last_colour);  // Start the line with the last colour tag we saw.
                length = word.size();   // Adjusts the length for what we have left over.
            }
            output.at(current_line) += word;
            line_pos += length;
        }
    }

    return output;
}

// Converts a string to a metadata map.
void StrX::string_to_metadata(const std::string &str, std::map<std::string, std::string> &metadata)
{
    metadata.clear();
    std::vector<std::string> md_exp = string_explode(str, " ");
    for (size_t i = 0; i < md_exp.size(); i++)
    {
        std::vector<std::string> md_pair = string_explode(md_exp.at(i), ":");
        if (md_pair.size() != 2) throw std::runtime_error("Corrupt metadata in string conversion.");
        metadata.insert(std::pair<std::string, std::string>(md_pair.at(0), md_pair.at(1)));
    }
}

// Strips colour codes from a string.
std::string StrX::strip_ansi(const std::string &str)
{
    std::regex filter("\\{[a-zA-Z0-9].?\\}");
    return std::regex_replace(str, filter, "");
}

// Returns the length of a string, taking colour and high/low-ASCII tags into account.
size_t StrX::strlen_colour(const std::string &str)
{
    size_t len = str.size();

    // Count any colour tags.
    len -= count_colour_tags(str) * 3;

    return len;
}

// Returns a time string as a rough description ("a few seconds", "a moment", "a few minutes").
std::string StrX::time_string_rough(float seconds)
{
    if (seconds < 1.0f) return "a moment";
    else if (seconds < 2.0f) return "a second";
    else if (seconds < 5.0f) return "a couple of seconds";
    else if (seconds < 10.0f) return "a few seconds";
    else if (seconds < 20.0f) return "about ten seconds";
    else if (seconds < 30.0f) return "about twenty seconds";
    else if (seconds < 60.0f) return "half a minute";
    else if (seconds < 120.0f) return "a minute";
    else if (seconds < 900.0f) return "a few minutes";
    else if (seconds < 1800.0f) return "a quarter of an hour";
    else if (seconds < 2700.0f) return "half an hour";
    else if (seconds < 3600.0f) return "three quarters of an hour";
    else if (seconds < 7200.0f) return "an hour";
    else if (seconds < 10800.0f) return "a couple of hours";
    else if (seconds < 43200.0f) return "several hours";
    else if (seconds < 82800.0f) return "half a day";
    else if (seconds < 86400.0f) return "most of a day";
    else if (seconds < 172800.0f) return "a day";
    else if (seconds < 259200.0f) return "a couple of days";
    else if (seconds < 604800.0f) return "several days";
    else if (seconds < 1209600.0f) return "a week";
    else if (seconds < 2592000.0f) return "weeks";
    else if (seconds < 5184000.0f) return "a month";
    else if (seconds < 31536000.0f) return "months";
    else if (seconds < 63072000.0f) return "a year";
    else return "years";
}

// Returns a count of the amount of times a string is found in a parent string.
size_t StrX::word_count(const std::string &str, const std::string &word)
{
    size_t count = 0;
    std::string::size_type word_pos = 0;
    while(word_pos != std::string::npos)
    {
        word_pos = str.find(word, word_pos);
        if (word_pos != std::string::npos)
        {
            count++;
            word_pos += word.length();
        }
    }
    return count;
}
