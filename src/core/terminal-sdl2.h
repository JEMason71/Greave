// core/terminal-sdl2.h -- Terminal interface for SDL2/SDL_ttf. See core/terminal.hpp for a full description of the Terminal class.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_CORE_TERMINAL_SDL2_H_
#define GREAVE_CORE_TERMINAL_SDL2_H_
#ifdef GREAVE_INCLUDE_SDL

#include "3rdparty/SDL2/SDL.h"
#include "3rdparty/SDL2/SDL_ttf.h"
#include "core/terminal.h"

#include <cstdint>
#include <map>
#include <string>


class TerminalSDL2 : public Terminal
{
public:
            TerminalSDL2();                             // Constructor, sets up SDL2.
            ~TerminalSDL2();                            // Destructor, cleans up SDL2.
    int     cell_height() const override;               // Returns the height of a single cell, in pixels.
    void    cls() override;                             // Clears the screen.
    void    cursor(bool visible) override;              // Makes the cursor visible or invisible.
    void    fill(int x, int y, int w, int h, Colour col = Colour::BLACK) override;  // Fills a given area in with the specified colour.
    int     get_key() override;                         // Gets keyboard input from the terminal.
    int     get_mouse_x() const override;               // Gets the X coordinate for the cell the mouse is pointing at.
    int     get_mouse_x_pixel() const override;         // Gets the X coordinate for the pixel the mouse is pointing at.
    int     get_mouse_y() const override;               // Gets the Y coordinate for the cell the mouse is pointing at.
    int     get_mouse_y_pixel() const override;         // Gets the Y coordinate for the pixel the mouse is pointing at.
    void    get_size(int *w, int *h) const override;    // Retrieves the size of the terminal (in cells, not pixels).
    void    move_cursor(int x, int y) override;         // Moves the cursor to the specified position.
    void    put(uint16_t letter, int x, int y, Colour col = Colour::WHITE) override;    // Prints a character at a given coordinate on the screen.
    void    refresh() override;                         // Refreshes the screen with changes made.
    bool    wants_to_close() const override;            // Returns true if the player has tried to close the SDL window.

private:
    struct RGB { uint8_t r, g, b; };

    void    colour_to_rgb(Colour col, uint8_t *r, uint8_t *g, uint8_t *b) const;    // Converts a colour code into a more useful form.

    void    init_colours();     // Loads the colours from prefs.yml into RGB values.
    void    print_internal(std::string str, int x, int y, Colour col) override;     // Internal rendering code, after print() has parsed the colour tags.
    void    screenshot();       // Takes a screenshot!

    std::map<Colour, RGB>   m_colour_map;           // Maps hex colours (e.g. FF2060) into individual RGB values.
    bool                    m_cursor_visible;       // Is the fake cursor visible?
    int                     m_cursor_x, m_cursor_y; // Coordinates for the fake cursor.
    TTF_Font*               m_font;                 // The font chosen by the user.
    int                     m_font_height;          // The height of the loaded font, in pixels.
    int                     m_font_width;           // The width of the loaded font, in pixels.
    bool                    m_init_sdl;             // SDL system successfully initialized.
    bool                    m_init_sdl_ttf;         // SDL_ttf system successfully initialized.
    int                     m_mouse_x;              // The X pixel coordinate of the mouse cursor's last location.
    int                     m_mouse_y;              // The Y pixel coordinate of the mouse cursor's last location.
    SDL_Renderer*           m_renderer;             // The SDL2 hardware renderer.
    time_t                  m_screenshot_msg_time;  // The timer for the screenshot message.
    int                     m_screenshot_taken;     // The last screenshot number taken.
    SDL_Window*             m_window;               // The one and only window the game uses.
    int                     m_window_h;             // The height of the window, in pixels.
    int                     m_window_w;             // The width of the window, in pixels.
};

#endif  // GREAVE_INCLUDE_SDL
#endif  // GREAVE_CORE_TERMINAL_SDL2_H_
