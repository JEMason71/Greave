// core/list.hpp -- Generic list of strings, which may or may not contain links to other lists.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. All rights reserved.

#pragma once
#include "core/greave.hpp"


struct ListEntry
{
    uint32_t    count;
    std::string str;
};

class List
{
public:
    ListEntry   at(uint32_t pos, bool nofollow = false) const;  // Returns the element at the given position of the List.
    bool        contains(const std::string &query) const;       // Checks to see if an entry exists on this List.
    void        merge_with(std::shared_ptr<List> second_list);  // Merges a second List into this List.
    void        push_back(ListEntry item);                      // Adds a new item to an existing List.
    ListEntry   rnd() const;                                    // Returns a random element from the List, parsing any sub-lists in the process.
    uint32_t    size() const;                                   // Returns the size of the List.

private:
    static const int    LIST_RARITY_UNCOMMON, LIST_RARITY_RARE, LIST_RARITY_SPECIAL;

    std::vector<ListEntry>  m_data;                             // The list's data, a vector.
};
