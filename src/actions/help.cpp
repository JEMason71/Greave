// actions/help.cpp -- The help command provides in-game documentation of commands and game mechanics.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/yaml-cpp/yaml.h"
#include "actions/help.hpp"
#include "core/core.hpp"
#include "core/strx.hpp"


std::map<std::string, std::string> ActionHelp::m_help_pages;    // Help pages loaded from data/misc/help.yml


// Asks for help on a specific topic.
void ActionHelp::help(std::string topic)
{
    if (!topic.size()) topic = "HELP";
    else topic = StrX::str_toupper(topic);
    StrX::find_and_replace(topic, " ", "_");

    const auto it = m_help_pages.find(topic);
    if (it == m_help_pages.end())
    {
        core()->message("{y}That help page does not exist. Type {Y}HELP {y}for an index.");
        return;
    }
    if (it->second.size() && it->second[0] == '#') help(it->second.substr(1));
    else core()->message(it->second);
}

// Loads the help pages from data/misc/help.yml
void ActionHelp::load_pages()
{
    try
    {
        const YAML::Node help_pages = YAML::LoadFile("data/misc/help.yml");
        for (const auto help_entry : help_pages)
        {
            std::string help_word = help_entry.first.as<std::string>(), help_text;
            if (help_entry.second.IsSequence())
            {
                for (unsigned int i = 0; i < help_entry.second.size(); i++)
                {
                    help_text += help_entry.second[i].as<std::string>();
                    if (i < help_entry.second.size() - 1) help_text += " {nl} ";
                }
            }
            else help_text = help_entry.second.as<std::string>();
            m_help_pages.insert(std::make_pair(help_word, help_text));
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Error while loading help data/misc/help.yml: " + std::string(e.what()));
    }
}
