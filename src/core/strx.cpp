// core/strx.cpp -- Various utility functions that deal with string manipulation/conversion.
// Copyright (c) 2009-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "core/core.hpp"
#include "core/guru.hpp"
#include "core/mathx.hpp"
#include "core/strx.hpp"

#include <algorithm>
#include <iterator>
#include <sstream>


const int StrX::CL_FLAG_USE_AND = 1, StrX::CL_FLAG_SQL_MODE = 2;    // comma_list() flags


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

std::string StrX::comma_list(std::vector<std::string> vec, unsigned int flags)
{
	const bool use_and = ((flags & CL_FLAG_USE_AND) == CL_FLAG_USE_AND);
	const bool sql_mode = ((flags & CL_FLAG_SQL_MODE) == CL_FLAG_SQL_MODE);
	if (!vec.size())
	{
		core()->guru()->nonfatal("Empty vector provided to comma_list!", Guru::WARN);
		return "";
	}
	if (vec.size() == 1) return vec.at(0);
	std::string plus = " and ";
	if (!use_and)
	{
		if (sql_mode) plus = ", ";
		else plus = " or ";
	}
	else if (vec.size() == 2) return vec.at(0) + plus + vec.at(1);

	std::string str;
	for (unsigned int i = 0; i < vec.size(); i++)
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
unsigned int StrX::count_colour_tags(const std::string &str)
{
    unsigned int tags = 0;
    for (unsigned int i = 0; i < str.size(); i++)
        if (str.at(i) == '{' && str.size() > i + 2 && str.at(i + 2) == '}') tags++;
    return tags;
}

// Converts a direction enum into a string.
std::string StrX::dir_to_name(Direction dir, DirNameType dnt)
{
    std::string prefix;
    if (dnt == DirNameType::TO_THE || dnt == DirNameType::TO_THE_ALT)
    {
        if (dir == Direction::UP) return (dnt == DirNameType::TO_THE ? "above" : "up");
        else if (dir == Direction::DOWN) return (dnt == DirNameType::TO_THE ? "below" : "down");
        else prefix = "to the ";
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
std::string StrX::ftos(double num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
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

// Checks if a string is a number.
bool StrX::is_number(const std::string &str)
{
    if (str.empty()) return false;
    size_t begin = 0;
    if (str[0] == '-') begin = 1;
    return std::all_of(str.begin() + begin, str.end(), ::isdigit);
}

// Converts an integer into a hex string.
std::string StrX::itoh(uint32_t num, unsigned int min_len)
{
    std::stringstream ss;
    ss << std::hex << num;
    std::string hex = ss.str();
    while (min_len && hex.size() < min_len) hex = "0" + hex;
    return hex;
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

// Calls MathX::round_to_two(), then returns the result as a string.
std::string StrX::round_to_two(double num) { return ftos(MathX::round_to_two(num)); }

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
    const int pit = separator.length();

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
std::vector<std::string> StrX::string_explode_colour(const std::string &str, unsigned int line_len)
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
            unsigned int length = word.length();    // Find the length of the word.

            const int colour_count = count_colour_tags(word);   // Count the colour tags.
            if (colour_count) length -= (colour_count * 3);     // Reduce the length if one or more colour tags are found.
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
    for (unsigned int i = 0; i < md_exp.size(); i++)
    {
        std::vector<std::string> md_pair = string_explode(md_exp.at(i), ":");
        if (md_pair.size() != 2) throw std::runtime_error("Corrupt metadata in string conversion.");
        metadata.insert(std::pair<std::string, std::string>(md_pair.at(0), md_pair.at(1)));
    }
}

// Strips colour codes from a string.
std::string StrX::strip_ansi(const std::string &str)
{
    std::string result = str;
    size_t pos;
    while ((pos = result.find("{")) != std::string::npos)
    {
        if (!pos) result = result.substr(3);
        else result = result.substr(0, pos) + result.substr(pos + 3);
    }
    return result;
}

// Returns the length of a string, taking colour and high/low-ASCII tags into account.
unsigned int StrX::strlen_colour(const std::string &str)
{
    unsigned int len = str.size();

    // Count any colour tags.
    len -= count_colour_tags(str) * 3;

    return len;
}

// Returns a count of the amount of times a string is found in a parent string.
unsigned int StrX::word_count(const std::string &str, const std::string &word)
{
    unsigned int count = 0;
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
