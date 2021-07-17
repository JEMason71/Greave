// core/prefs.h -- The Prefs class loads data from prefs.yml, allowing the user to configure game settings.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_CORE_PREFS_H_
#define GREAVE_CORE_PREFS_H_

#include <string>


namespace greave {

class Prefs
{
public:
                    Prefs();                // Constructor, loads data from prefs.yml

    std::string colour_black;           // Hex colour definition for black.
    std::string colour_blue;            // Hex colour definition for bold blue.
    std::string colour_blue_dark;       // Hex colour definition for dark blue.
    std::string colour_cyan;            // Hex colour definition for bold cyan.
    std::string colour_cyan_dark;       // Hex colour definition for dark cyan.
    std::string colour_green;           // Hex colour definition for bold green.
    std::string colour_green_dark;      // Hex colour definition for dark green.
    std::string colour_grey;            // Hex colour definition for grey.
    std::string colour_grey_dark;       // Hex colour definition for dark grey.
    std::string colour_grey_darkest;    // This colour is used for the background of the console window on SDL.
    std::string colour_magenta;         // Hex colour definition for bold magenta.
    std::string colour_magenta_dark;    // Hex colour definition for dark magenta.
    std::string colour_red;             // Hex colour definition for bold red.
    std::string colour_red_dark;        // Hex colour definition for dark red.
    std::string colour_white;           // Hex colour definition for white.
    std::string colour_yellow;          // Hex colour definition for bold yellow.
    std::string colour_yellow_dark;     // Hex colour definition for dark yellow.
    bool        curses_custom_colours;  // Apply custom colour values above to Curses colours.
    int         log_max_size;           // How many lines of text to keep in the message log?
    int         log_mouse_scroll_step;  // How many lines to scroll the window, when using the mouse-wheel.
    int         log_padding_bottom;     // The amount of black space below the message log window. (Must be at least 2, or the input box will be hidden.)
    int         log_padding_left;       // The amount of black space to the left of the message log window.
    int         log_padding_right;      // The amount of black space to the right of the message log window.
    int         log_padding_top;        // The amount of black space above the message log window.
    bool        monochrome_mode;        // Set this to true to only use black/gray for the background and white for the text.
    int         save_file_slots;        // The total amount of saved game slots available.
#ifdef GREAVE_TOLK
    bool        screen_reader_external; // Enable automatic screen-reader support? Screen readers supported: JAWS, NVDA, SuperNova, System Access, Window-Eyes, ZoomText.
    bool        screen_reader_process_square_brackets;  // This setting can improve narration on screen readers for square brackets.
    bool        screen_reader_sapi;     // Enable this to default to Microsoft SAPI text-to-speech, without using any external screen-reader software.
#endif
    std::string sdl_console_size;       // The number of columns and rows the window should be sized to by default.
    std::string sdl_font;               // The TTF font used in the game (must be monospace).
    int         sdl_font_size;          // The size of the main font.
    bool        sdl_vsync;              // Should the game use vsync?
    std::string terminal;               // Set this to sdl2 for SDL2, or curses for PDCurses/NCurses.
};

}       // namespace greave
#endif  // GREAVE_CORE_PREFS_H_
