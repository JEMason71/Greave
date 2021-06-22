// core/utility.hpp -- Miscellaneous utility functions which don't really fit into any one specific place.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "uni/uni-system.hpp"


class Util
{
public:
    static void delete_file(const std::string &filename);   // Deletes a specified file.
    static bool directory_exists(const std::string &dir);   // Check if a directory exists.
    static bool file_exists(const std::string &file);       // Checks if a file exists.
    static void make_dir(const std::string &dir);           // Makes a new directory, if it doesn't already exist.
};
