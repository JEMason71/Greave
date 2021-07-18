// actions/help.h -- The help command provides in-game documentation of commands and game mechanics.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_ACTIONS_HELP_H_
#define GREAVE_ACTIONS_HELP_H_

#include <map>
#include <string>


class ActionHelp
{
public:
    static void help(std::string topic);    // Asks for help on a specific topic.
    static void load_pages();               // Loads the help pages from data/misc/help.yml

private:
    static std::map<std::string, std::string>   help_pages_;    // Help pages loaded from data/misc/help.yml
};

#endif  // GREAVE_ACTIONS_HELP_H_
