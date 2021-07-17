// core/filex.h -- Various utility functions that deal with creating, deleting, and manipulating files.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_CORE_FILEX_H_
#define GREAVE_CORE_FILEX_H_

#include <string>
#include <vector>


namespace greave {

class FileX
{
public:
    static void delete_file(const std::string &filename);   // Deletes a specified file.
    static bool directory_exists(const std::string &dir);   // Check if a directory exists.
    static bool file_exists(const std::string &file);       // Checks if a file exists.
    static std::vector<std::string> files_in_dir(const std::string &directory, bool recursive = false); // Returns a list of files in a given directory.
    static bool is_read_only(const std::string &file);      // Checks if a file is read-only.
    static void make_dir(const std::string &dir);           // Makes a new directory, if it doesn't already exist.
    static void rename_file(const std::string &old_name, const std::string &new_name);  // Renames a file.
};

}       // namespace greave
#endif  // GREAVE_CORE_FILEX_H_
