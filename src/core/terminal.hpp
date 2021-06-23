// core/terminal.hpp -- Middleware layer between the game proper and the terminal emulator being used (Curses, BLT, etc.)
// This way, multiple alternative terminal emulators can be plugged in without affecting the rest of the code.
// The base Terminal class is pure virtual; derived classes should handle code specific to their specific terminal emulator.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


class Terminal
{
public:
    enum class Colour : uint8_t { BLACK, BLACK_BOLD, RED, RED_BOLD, GREEN, GREEN_BOLD, YELLOW, YELLOW_BOLD, BLUE, BLUE_BOLD, MAGENTA, MAGENTA_BOLD, CYAN, CYAN_BOLD, WHITE, WHITE_BOLD,
        WHITE_BG };
    enum Key { BACKSPACE = 8, TAB = 9, LF = 10, CR = 13, CLOSE = 256, RESIZED, ARROW_UP, ARROW_DOWN, ARROW_LEFT, ARROW_RIGHT, HOME, END, PAGE_UP, PAGE_DOWN, MOUSE_SCROLL_UP,
        MOUSE_SCROLL_DOWN, MOUSE_LEFT, MOUSE_LEFT_RELEASED, MOUSE_MOVED };

    virtual             ~Terminal() { }                     // Virtual destructor, should clean up any terminal emulator-specific memory/states.
    virtual int         cell_height() const = 0;            // Returns the height of a single cell, in pixels.
    virtual void        cls() = 0;                          // Clears the screen.
    virtual void        cursor(bool visible) = 0;           // Makes the cursor visible or invisible.
    virtual void        fill(int x, int y, int w, int h, Colour col = Colour::BLACK) = 0;   // Fills a given area in with the specified colour.
    virtual int         get_key() const = 0;                // Gets keyboard input from the terminal.
    virtual int         get_mouse_x() const = 0;            // Gets the X coordinate for the cell the mouse is pointing at.
    virtual int         get_mouse_x_pixel() const = 0;      // Gets the X coordinate for the pixel the mouse is pointing at.
    virtual int         get_mouse_y() const = 0;            // Gets the Y coordinate for the cell the mouse is pointing at.
    virtual int         get_mouse_y_pixel() const = 0;      // Gets the Y coordinate for the pixel the mouse is pointing at.
    virtual void        get_size(int *w, int *h) const = 0; // Retrieves the size of the terminal (in cells, not pixels).
    virtual void        move_cursor(int x, int y) = 0;      // Moves the cursor to the specified position.
    virtual void        print(std::string str, int x, int y, Colour col = Colour::WHITE_BOLD) = 0;      // Prints a string at a given coordinate on the screen.
    virtual void        put(unsigned int letter, int x, int y, Colour col = Colour::WHITE_BOLD) = 0;    // Prints a character at a given coordinate on the screen.
    virtual void        refresh() = 0;                      // Refreshes the screen with changes made.
    virtual void        set_background(Colour col = Colour::BLACK) = 0; // Sets the text background colour.
    virtual bool        wants_to_close() const = 0;         // Returns true if the player has tried to close the terminal window.
};
