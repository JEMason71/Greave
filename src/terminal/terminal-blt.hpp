// terminal/terminal-blt.hpp -- Terminal interface for BearLibTerminal. See terminal.h for a full description of the Terminal class.
// Copyright (c) 2020 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "terminal/terminal.hpp"


class TerminalBLT : public Terminal
{
public:
                TerminalBLT();      // Constructor, sets up BearLibTerminal.
                ~TerminalBLT();     // Destructor, cleans up BearLibTerminal.
    int         cell_height() override; // Returns the height of a single cell, in pixels.
    void        cls() override;     // Clears the screen.
    void        cursor(bool visible) override;  // Makes the cursor visible or invisible.
    void        fill(int x, int y, int w, int h, Colour col = Colour::BLACK) override;  // Fills a given area in with the specified colour.
    int         get_key() override; // Gets keyboard input from the terminal.
    int         get_mouse_x() override;         // Gets the X coordinate for the cell the mouse is pointing at.
    int         get_mouse_x_pixel() override;   // Gets the X coordinate for the pixel the mouse is pointing at.
    int         get_mouse_y() override;         // Gets the Y coordinate for the cell the mouse is pointing at.
    int         get_mouse_y_pixel() override;   // Gets the Y coordinate for the pixel the mouse is pointing at.
    void        get_size(int *w, int *h) override;  // Retrieves the size of the terminal (in cells, not pixels).
    void        move_cursor(int x, int y) override; // Moves the cursor to the specified position.
    void        print(std::string str, int x, int y, Colour col = Colour::WHITE_BOLD) override;     // Prints a string at a given coordinate on the screen.
    void        put(unsigned int letter, int x, int y, Colour col = Colour::WHITE_BOLD) override;   // Prints a character at a given coordinate on the screen.
    void        refresh() override; // Refreshes the screen with changes made.
    void        set_background(Colour col = Colour::BLACK) override;    // Sets the text background colour.
    bool        wants_to_close() override;  // Returns true if the terminal window has been closed.

private:
    std::string background_colour();    // The defined background window colour for BLT.
    std::string colour(Colour col);     // Converts a Terminal::Colour into a BLT colour string.

    bool    m_cursor_visible;       // Is the fake cursor visible?
    int     m_cursor_x, m_cursor_y; // Coordinates for the fake cursor.
};
