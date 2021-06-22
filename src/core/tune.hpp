// core/tune.hpp -- The Tune class loads data from tune.yml, allowing for various numbers to be tweaked on-the-fly.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "uni/uni-system.hpp"


class Tune
{
public:
        Tune(); // Constructor, loads data from tune.yml

    std::string	blt_console_size;       // The number of columns and rows the window should be sized to by default.
    std::string	blt_font;               // The TTF font used in the game (must be monospace).
    int         blt_font_size;          // The size of the main font.
    std::string	blt_log_file;           // The filename for the BearLibTerminal log file in the userdata folder.
    std::string	blt_log_level;          // The minimum level of logging for BearLibTerminal.
    bool        blt_vsync;              // Should the game use vsync?
    std::string	colour_blue;            // Hex colour definition for blue.
    std::string	colour_cyan;            // Hex colour definition for cyan.
    std::string	colour_green;           // Hex colour definition for green.
    std::string	colour_grey;            // Hex colour definition for grey.
    std::string	colour_magenta;         // Hex colour definition for magenta.
    std::string	colour_orange;          // Hex colour definition for orange.
    std::string	colour_purple;          // Hex colour definition for purple.
    std::string	colour_red;             // Hex colour definition for red.
    std::string	colour_yellow;          // Hex colour definition for yellow.
    std::string	colour_white;           // Hex colour definition for white.
    bool        curses_custom_colours;  // Apply custom colour values above to Curses colours.
    bool        monochrome_mode;        // Set this to true to only use black/gray for the background and white for the text.
#ifdef GREAVE_TOLK
    bool        screen_reader_external; // Enable automatic screen-reader support? Screen readers supported: JAWS, NVDA, SuperNova, System Access, Window-Eyes, ZoomText.
    bool        screen_reader_process_square_brackets;  // This setting can improve narration on screen readers for square brackets.
    bool        screen_reader_sapi;     // Enable this to default to Microsoft SAPI text-to-speech, without using any external screen-reader software.
#endif
    std::string terminal;               // Set this to blt for BearLibTerminal, or curses for PDCurses/NCurses.
};
