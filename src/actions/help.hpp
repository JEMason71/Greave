// actions/help.hpp -- The help command provides in-game documentation of commands and game mechanics.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


class ActionHelp
{
public:
    static void help(std::string topic);    // Asks for help on a specific topic.
    static void load_pages();               // Loads the help pages from data/help.yml

private:
    static std::map<std::string, std::string>   m_help_pages;   // Help pages loaded from data/help.yml
};
