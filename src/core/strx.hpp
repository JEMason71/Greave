// core/strx.hpp -- Various utility functions that deal with string manipulation/conversion.
// Copyright (c) 2009-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/core.hpp"

enum class Direction : uint8_t; // defined in world/room.hpp


class StrX
{
public:
    static const int    CL_FLAG_USE_AND, CL_FLAG_SQL_MODE;  // comma_list() flags

    enum class DirNameType : uint8_t { NORMAL, TO_THE, TO_THE_ALT, FROM_THE, FROM_THE_ALT };
    enum class MGSC : uint8_t { SHORT, SHORT_ROUND, LONG }; // mgsc_string() modes.

    static std::string  capitalize_first_letter(std::string str);   // Capitalizes the first letter of a string.
    static std::string  collapse_vector(std::vector<std::string> vec);  // Simple function to collapse a string vector into words.
    static std::string  collapse_vector(std::vector<uint32_t> vec);     // As above, but for an integer vector>
    static std::string  comma_list(std::vector<std::string> vec, int flags = 0);    // Converts a vector to a comma-separated list.
    static size_t       count_colour_tags(const std::string &str);  // Counts all the colour tags in a string.
    static std::string  decode_compressed_string(std::string cb);   // Decodes a compressed string (e.g. 4cab2z becomes ccccabzz).
    static std::string  dir_to_name(Direction dir, DirNameType dnt = DirNameType::NORMAL);  // Converts a direction enum into a string.
    static std::string  dir_to_name(uint8_t dir, DirNameType dnt = DirNameType::NORMAL);    // As above, but with an integer instead of an enum.
    static bool         find_and_replace(std::string &input, const std::string &to_find, const std::string &to_replace);    // Find and replace one string with another.
    static std::string  ftos(double num, bool force_decimal = false);   // Converts a float or double to a string.
    static uint32_t     hash(const std::string &str);               // FNV string hash function.
    static uint32_t     htoi(const std::string &hex_str);           // Converts a hex string back to an integer.
    static std::string  intostr_pretty(int num);                    // Returns a 'pretty' version of a number in string format, such as "12,345".
    static bool         is_number(const std::string &str);          // Checks if a string is a number.
    static bool         is_vowel(char ch);                          // Checks if a character is a vowel.
    static std::string  itoh(uint32_t num, size_t min_len);         // Converts an integer into a hex string.
    static std::string  itos(uint32_t num, size_t min_len);         // Converts an integer to a string, but optionally pads it to a minimum length with leading zeroes.
    static std::string  metadata_to_string(const std::map<std::string, std::string> &metadata); // Converts a metadata map into a string.
    static std::string  mgsc_string(uint32_t coin, MGSC mode);      // Converts a coin value into a mithril/gold/silver/copper ANSI string.
    static std::string  number_to_word(uint64_t number);            // Converts small numbers into words.
    static std::string  possessive_string(const std::string &str);  // Makes a string into a possessive noun (e.g. orc = orc's, platypus = platypus')
    static std::string  rainbow_text(const std::string &str, const std::string &colours);   // Makes pretty rainbow text!
    static std::string  round_to_two(double num);                   // Calls MathX::round_to_two(), then returns the result as a string.
    static std::vector<uint32_t>    stoi_vec(std::vector<std::string> vec); // Converts a std::string vector into a uint32_t vector.
    static std::string  str_tolower(std::string str);               // Converts a string to lower-case.
    static std::string  str_toupper(std::string str);               // Converts a string to upper-case.
    static std::vector<std::string> string_explode(std::string str, const std::string &separator);  // String split/explode function.
    static std::vector<std::string> string_explode_colour(const std::string &str, size_t line_len); // Similar to string_explode(), but takes colour into account, and wraps to a given line.
    static void         string_to_metadata(const std::string &str, std::map<std::string, std::string> &metadata);   // Converts a string to a metadata map.
    static std::string  strip_ansi(const std::string &str);         // Strips colour codes from a string.
    static size_t       strlen_colour(const std::string &str);      // Returns the length of a string, taking colour tags into account.
    static size_t       word_count(const std::string &str, const std::string &word);    // Returns a count of the amount of times a string is found in a parent string.

    template<class T> static void string_to_tags(const std::string &tag_string, std::set<T> &tags)
    {
        if (!tag_string.size()) return;
        std::vector<std::string> split_tags = string_explode(tag_string, " ");
        for (auto tag : split_tags)
            tags.insert(static_cast<T>(htoi(tag)));
    }

    template<class T> static std::string tags_to_string(std::set<T> tags)
    {
        if (!tags.size()) return "";
        std::string tags_str;
        for (auto tag : tags)
            if (static_cast<uint32_t>(tag) < Core::TAGS_PERMANENT) tags_str += itoh(static_cast<long long>(tag), 1) + " ";
        if (tags_str.size()) tags_str.pop_back();   // Strip off the excess space at the end.
        return tags_str;
    }
};
