// core/tune.hpp -- The Tune class loads data from tune.yml, allowing for various numbers to be tweaked on-the-fly.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "uni/uni-system.hpp"


class Tune
{
public:
        Tune(); // Constructor, loads data from tune.yml

#ifdef GREAVE_TOLK
    bool    screen_reader_external; // Enable automatic screen-reader support? Screen readers supported: JAWS, NVDA, SuperNova, System Access, Window-Eyes, ZoomText.
    bool    screen_reader_process_square_brackets;  // This setting can improve narration on screen readers for square brackets.
    bool    screen_reader_sapi;     // Enable this to default to Microsoft SAPI text-to-speech, without using any external screen-reader software.
#endif
};
