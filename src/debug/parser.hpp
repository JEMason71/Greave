// debug/parser.hpp -- The debug parser, handles debug/testing/cheat commands starting with #.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


class DebugParser
{
public:
    void    parse(std::vector<std::string> words);  // Parses debug commands.
};
