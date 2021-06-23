// core/terminal-curses.hpp -- Terminal interface for PDCurses/NCurses. See core/terminal.hpp for a full description of the Terminal class.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/terminal.hpp"


class TerminalCurses : public Terminal
{
public:
                TerminalCurses();   // Constructor, sets up Curses.
                ~TerminalCurses();  // Destructor, cleans up Curses.
    int         cell_height() override { return 0; }    // Returns the height of a single cell, in pixels. Not used in Curses.
    void        cls() override;     // Clears the screen.
    void        cursor(bool visible) override;  // Makes the cursor visible or invisible.
    void        fill(int x, int y, int w, int h, Colour col = Colour::BLACK) override;  // Fills a given area in with the specified colour. Currently nonfunctional on Curses.
    int         get_key() override; // Gets keyboard input from the terminal.
    int         get_mouse_x() override { return 0; }        // Not currently supported by the Curses interface.
    int         get_mouse_x_pixel() override { return 0; }  // Not currently supported by the Curses interface.
    int         get_mouse_y() override { return 0; }        // Not currently supported by the Curses interface.
    int         get_mouse_y_pixel() override { return 0; }  // Not currently supported by the Curses interface.
    void        get_size(int *w, int *h) override;  // Retrieves the size of the terminal (in cells, not pixels).
    void        move_cursor(int x, int y) override; // Moves the cursor to the specified position.
    void        print(std::string str, int x, int y, Colour col = Colour::WHITE_BOLD) override;     // Prints a string at a given coordinate on the screen.
    void        put(unsigned int letter, int x, int y, Colour col = Colour::WHITE_BOLD) override;   // Prints a character at a given coordinate on the screen.
    void        refresh() override; // Refreshes the screen with changes made.
    void        set_background(Colour col = Colour::BLACK) override;    // Sets the text background colour. Currently nonfunctional on Curses.
    bool        wants_to_close() override;  // Returns true if the player uses Ctrl-C, Ctrl-D or escape.

private:
    unsigned long   colour(Colour col); // Returns a colour pair code.
    static void     decode_hex_colour(const std::string &col, short &r, short &g, short &b);    // Decodes a hex-code colour into RGB values.

    enum CustomColour { CUSTOM_BLACK = 100, CUSTOM_GREY_DARK, CUSTOM_RED, CUSTOM_RED_DARK, CUSTOM_GREEN, CUSTOM_GREEN_DARK, CUSTOM_YELLOW, CUSTOM_YELLOW_DARK, CUSTOM_BLUE,
        CUSTOM_BLUE_DARK, CUSTOM_CYAN, CUSTOM_CYAN_DARK, CUSTOM_MAGENTA, CUSTOM_MAGENTA_DARK, CUSTOM_WHITE, CUSTOM_GREY, CUSTOM_WHITE_BG };
};
