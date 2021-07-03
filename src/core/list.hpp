// core/list.hpp -- Generic list of strings, which may or may not contain links to other lists.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. All rights reserved.

#pragma once
#include "core/greave.hpp"


struct ListEntry
{
                ListEntry() { };
                ListEntry(const std::string &le_str, uint32_t le_level, uint32_t le_count) : count(le_count), level(le_level), str(le_str) { }
    uint32_t    count, level;
    std::string str;
};

class List
{
public:
    ListEntry   at(uint32_t pos, int flags = 0) const;          // Returns the element at the given position of the List.
    bool        contains(const std::string &query) const;       // Checks to see if an entry exists on this List.
    void        push_back(ListEntry item);                      // Adds a new item to an existing List.
    ListEntry   rnd(uint32_t level = 0, int flags = 0) const;   // Returns a random element from the List, parsing any sub-lists in the process.
    uint32_t    size() const;                                   // Returns the size of the List.

    static const int    LIST_FLAG_NO_FOLLOW;                    // Flags for at() and rnd() functions.

private:
    void        merge_with(std::shared_ptr<List> second_list);  // Merges a second List into this List.
    void        prune(uint32_t level);                          // Prunes any list entries where the level is not the specified level.

    std::vector<ListEntry>  m_data;                             // The list's data, a vector.
};
