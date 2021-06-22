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
