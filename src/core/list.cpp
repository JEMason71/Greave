// core/list.cpp -- Generic list of strings, which may or may not contain links to other lists.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. All rights reserved.

#include "core/core.hpp"
#include "core/list.hpp"
#include "core/random.hpp"
#include "world/world.hpp"


// Flags for at() and rnd() functions.
const int List::LIST_FLAG_NO_FOLLOW =   1;  // Ignore linked lists.


// Returns the element at the given position of the List.
ListEntry List::at(uint32_t pos, int flags) const
{
    const bool nofollow = (flags & LIST_FLAG_NO_FOLLOW) == LIST_FLAG_NO_FOLLOW;

    if (pos >= m_data.size())
    {
        throw std::runtime_error("Invalid list position: " + std::to_string(pos));
        ListEntry blank("", 0, 0);
        return blank;
    }
    if (m_data.at(pos).str[0] == '#' && !nofollow) return core()->world()->get_list(m_data.at(pos).str.substr(1)).rnd(0, flags);
    return m_data.at(pos);
}

// Checks to see if an entry exists on this List.
bool List::contains(const std::string &query) const
{
    for (auto le : m_data)
    {
        std::string list_data = le.str;
        if (!list_data.size()) continue;
        if (list_data[0] == '#' && core()->world()->get_list(list_data.substr(1)).contains(query)) return true;
        else if (list_data == query) return true;
    }
    return false;
}

// Merges a second List into this List.
void List::merge_with(std::shared_ptr<List> second_list)
{
    for (uint32_t i = 0; i < second_list->size(); i++)
    {
        ListEntry new_entry = second_list->at(i, LIST_FLAG_NO_FOLLOW);
        m_data.push_back(new_entry);
    }
}

// Prunes any list entries where the level is not the specified level.
void List::prune(uint32_t level)
{
    if (!level) return;
    for (int i = m_data.size() - 1; i >= 0; i--)
    {
        if (m_data.at(i).level != level)
        {
            if (!m_data.at(i).str.size() || m_data.at(i).str[0] != '#')
                m_data.erase(m_data.begin() + i);
        }
    }
}

// Adds a new item to an existing List.
void List::push_back(ListEntry item) { m_data.push_back(item); }

// Returns a random element from the list, parsing any sub-lists in the process.
ListEntry List::rnd(uint32_t level, int flags) const
{
    const bool nofollow = (flags & LIST_FLAG_NO_FOLLOW) == LIST_FLAG_NO_FOLLOW;

    auto list_copy = std::make_shared<List>(*this);
    if (level > 0) list_copy->prune(level);
    while (true)
    {
        if (!list_copy->size()) throw std::runtime_error("Could not find suitable result on list.");
        const uint32_t choice = core()->rng()->rnd(0, list_copy->m_data.size() - 1);
        ListEntry result = list_copy->m_data.at(choice);
        if (result.str.size() && result.str[0] == '#' && !nofollow)
        {
            List linked_list = core()->world()->get_list(result.str.substr(1));
            if (level > 0) linked_list.prune(level);
            if (!linked_list.size())
            {
                list_copy->m_data.erase(list_copy->m_data.begin() + choice);
                continue;
            }
            return linked_list.rnd(level, flags);
        }
        else return result;
    }
}

// Returns the size of the List.
uint32_t List::size() const { return m_data.size(); }
