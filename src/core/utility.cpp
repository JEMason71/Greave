// core/utility.cpp -- Miscellaneous utility functions which don't really fit into any one specific place.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "uni/uni-core.hpp"

#include <algorithm>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>


// Deletes a specified file. Simple enough, but we'll keep this function around in case there's any platform-specific weirdness that needs to be worked in.
void Util::delete_file(const std::string &filename) { unlink(filename.c_str()); }

// Check if a directory exists.
bool Util::directory_exists(const std::string &dir)
{
    struct stat info;
    if (stat(dir.c_str(), &info) != 0) return false;
    if (info.st_mode & S_IFDIR) return true;
    return false;
}

// Checks if a file exists.
bool Util::file_exists(const std::string &file)
{
    struct stat info;
    return (stat(file.c_str(), &info) == 0);
}

// Find and replace one string with another.
bool Util::find_and_replace(std::string &input, const std::string &to_find, const std::string &to_replace)
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

// Converts a hex string back to an integer.
uint32_t Util::htoi(const std::string &hex_str)
{
    std::stringstream ss;
    ss << std::hex << hex_str;
    uint32_t result;
    ss >> result;
    return result;
}

// Makes a new directory, if it doesn't already exist.
void Util::make_dir(const std::string &dir)
{
    if (directory_exists(dir)) return;

#ifdef GREAVE_TARGET_WINDOWS
    mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0777);
#endif
}

// Converts a string to lower-case.
std::string Util::str_tolower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

// String split/explode function.
std::vector<std::string> Util::string_explode(std::string str, const std::string &separator)
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
std::vector<std::string> Util::string_explode_colour(const std::string &str, unsigned int line_len)
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
    if (strlen_colour(str) <= line_len)
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
        unsigned int length = word.length();    // Find the length of the word.

        const int colour_count = word_count(word, "{"); // Count the colour tags.
        if (colour_count) length -= (colour_count * 3); // Reduce the length if one or more colour tags are found.
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

    return output;
}

// Returns the length of a string, taking colour and high/low-ASCII tags into account.
unsigned int Util::strlen_colour(const std::string &str)
{
    unsigned int len = str.size();

    // Count any colour tags.
    const int openers = std::count(str.begin(), str.end(), '{');
    if (openers) len -= openers * 3;

    return len;
}

// Returns a count of the amount of times a string is found in a parent string.
unsigned int Util::word_count(const std::string &str, const std::string &word)
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
