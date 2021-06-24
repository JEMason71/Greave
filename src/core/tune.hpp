// core/tune.hpp -- The Tune class loads data from tune.yml, allowing for various numbers to be tweaked on-the-fly.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


class Tune
{
public:
        Tune(); // Constructor, loads data from tune.yml

    std::string     blt_console_size;       // The number of columns and rows the window should be sized to by default.
    std::string     blt_font;               // The TTF font used in the game (must be monospace).
    unsigned int    blt_font_size;          // The size of the main font.
    std::string     blt_log_file;           // The filename for the BearLibTerminal log file in the userdata folder.
    std::string     blt_log_level;          // The minimum level of logging for BearLibTerminal.
    bool            blt_vsync;              // Should the game use vsync?
    std::string     colour_black;           // Hex colour definition for black.
    std::string     colour_blue;            // Hex colour definition for bold blue.
    std::string     colour_blue_dark;       // Hex colour definition for dark blue.
    std::string     colour_cyan;            // Hex colour definition for bold cyan.
    std::string     colour_cyan_dark;       // Hex colour definition for dark cyan.
    std::string     colour_green;           // Hex colour definition for bold green.
    std::string     colour_green_dark;      // Hex colour definition for dark green.
    std::string     colour_grey;            // Hex colour definition for grey.
    std::string     colour_grey_dark;       // Hex colour definition for dark grey.
    std::string     colour_magenta;         // Hex colour definition for bold magenta.
    std::string     colour_magenta_dark;    // Hex colour definition for dark magenta.
    std::string     colour_red;             // Hex colour definition for bold red.
    std::string     colour_red_dark;        // Hex colour definition for dark red.
    std::string     colour_white;           // Hex colour definition for white.
    std::string     colour_yellow;          // Hex colour definition for bold yellow.
    std::string     colour_yellow_dark;     // Hex colour definition for dark yellow.
    bool            curses_custom_colours;  // Apply custom colour values above to Curses colours.
    unsigned int    log_max_size;           // How many lines of text to keep in the message log?
    unsigned int    log_mouse_scroll_step;  // How many lines to scroll the window, when using the mouse-wheel.
    int             log_padding_bottom;     // The amount of black space below the message log window. (Must be at least 2, or the input box will be hidden.)
    int             log_padding_left;       // The amount of black space to the left of the message log window.
    int             log_padding_right;      // The amount of black space to the right of the message log window.
    int             log_padding_top;        // The amount of black space above the message log window.
    bool            monochrome_mode;        // Set this to true to only use black/gray for the background and white for the text.
    unsigned int    save_file_slots;        // The total amount of saved game slots available.
#ifdef GREAVE_TOLK
    bool            screen_reader_external; // Enable automatic screen-reader support? Screen readers supported: JAWS, NVDA, SuperNova, System Access, Window-Eyes, ZoomText.
    bool            screen_reader_process_square_brackets;  // This setting can improve narration on screen readers for square brackets.
    bool            screen_reader_sapi;     // Enable this to default to Microsoft SAPI text-to-speech, without using any external screen-reader software.
#endif
    std::string     terminal;               // Set this to blt for BearLibTerminal, or curses for PDCurses/NCurses.
};
