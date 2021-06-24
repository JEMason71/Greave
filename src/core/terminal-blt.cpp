// core/terminal-blt.cpp -- Terminal interface for BearLibTerminal. See core/terminal.hpp for a full description of the Terminal class.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/BearLibTerminal/BearLibTerminal.h"
#include "core/core.hpp"
#include "core/filex.hpp"
#include "core/guru.hpp"
#include "core/strx.hpp"
#include "core/terminal-blt.hpp"
#include "core/tune.hpp"


// Constructor, sets up BearLibTerminal.
TerminalBLT::TerminalBLT() : m_cursor_visible(false), m_cursor_x(0), m_cursor_y(0)
{
    const std::shared_ptr<Tune> tune = core()->tune();
    std::string ver_str = Core::GAME_VERSION;

    core()->guru()->log("Setting up BearLibTerminal.");
    FileX::delete_file("userdata/" + tune->blt_log_file);
    terminal_open();
    terminal_set((
        "window: size = " + tune->blt_console_size + ", title = 'Greave " + ver_str + "', icon = 'data/app-icon.ico', resizeable = true, fullscreen = false, vsync = " +
            (tune->blt_vsync ? "true" : "false") + "; "
        "log: file = 'userdata/" + tune->blt_log_file + "', level = " + tune->blt_log_level + "; "
        "input: filter = 'keyboard, mouse+', precise-mouse = true; "
        "font: 'data/" + tune->blt_font + "', size = " + std::to_string(tune->blt_font_size) + "; "
        "palette.g_black = #" + tune->colour_black + "; "
        "palette.g_blue = #" + tune->colour_blue + "; "
        "palette.g_blue_dark = #" + tune->colour_blue_dark + "; "
        "palette.g_cyan = #" + tune->colour_cyan + "; "
        "palette.g_cyan_dark = #" + tune->colour_cyan_dark + "; "
        "palette.g_green = #" + tune->colour_green + "; "
        "palette.g_green_dark = #" + tune->colour_green_dark + "; "
        "palette.g_grey = #" + tune->colour_grey + "; "
        "palette.g_grey_dark = #" + tune->colour_grey_dark + "; "
        "palette.g_magenta = #" + tune->colour_magenta + "; "
        "palette.g_magenta_dark = #" + tune->colour_magenta_dark + "; "
        "palette.g_red = #" + tune->colour_red + "; "
        "palette.g_red_dark = #" + tune->colour_red_dark + "; "
        "palette.g_white = #" + tune->colour_white + "; "
        "palette.g_yellow = #" + tune->colour_yellow + "; "
        "palette.g_yellow_dark = #" + tune->colour_yellow_dark + "; "
        ).c_str());
}

// Destructor, cleans up BearLibTerminal.
TerminalBLT::~TerminalBLT() { terminal_close(); }

// The defined background window colour for BLT.
std::string TerminalBLT::background_colour() const { return "black"; }

// Returns the height of a single cell, in pixels.
int TerminalBLT::cell_height() const { return terminal_state(TK_CELL_HEIGHT); }

void TerminalBLT::cls()
{
    terminal_bkcolor(background_colour().c_str());
    terminal_color(colour(Colour::WHITE).c_str());
    terminal_clear();
}

// Converts a Terminal::Colour into a BLT colour string.
std::string TerminalBLT::colour(Colour col) const
{
    if (core()->tune()->monochrome_mode) switch(col)
    {
        case Colour::BLACK: case Colour::WHITE_BG: return "black";
        case Colour::BLACK_BOLD: return "darkest grey";
        case Colour::RED: case Colour::GREEN: case Colour::YELLOW: case Colour::BLUE: case Colour::MAGENTA: case Colour::CYAN: case Colour::WHITE: return "lighter grey";
        case Colour::RED_BOLD: case Colour::GREEN_BOLD: case Colour::YELLOW_BOLD: case Colour::BLUE_BOLD: case Colour::MAGENTA_BOLD: case Colour::CYAN_BOLD: case Colour::WHITE_BOLD:
            return "white";
        default: return "";
    }
    else switch(col)
    {
        case Colour::BLACK: case Colour::WHITE_BG: return "black";
        case Colour::BLACK_BOLD: return "#101010";
        case Colour::RED: return "darker red";
        case Colour::RED_BOLD: return "g_red";
        case Colour::GREEN: return "darker green";
        case Colour::GREEN_BOLD: return "g_green";
        case Colour::YELLOW: return "g_orange";
        case Colour::YELLOW_BOLD: return "g_yellow";
        case Colour::BLUE: return "dark azure";
        case Colour::BLUE_BOLD: return "g_blue";
        case Colour::MAGENTA: return "g_purple";
        case Colour::MAGENTA_BOLD: return "g_magenta";
        case Colour::CYAN: return "darker cyan";
        case Colour::CYAN_BOLD: return "g_cyan";
        case Colour::WHITE: return "g_grey";
        case Colour::WHITE_BOLD: return "g_white";
        default: return "";
    }
}

// Makes the cursor visible or invisible.
void TerminalBLT::cursor(bool visible) { m_cursor_visible = visible; }

// Fills a given area in with the specified colour.
void TerminalBLT::fill(int x, int y, int w, int h, Colour col)
{
    terminal_bkcolor(colour(col).c_str());
    terminal_clear_area(x, y, w, h);
}

// Gets keyboard input from the terminal.
int TerminalBLT::get_key() const
{
    const int key = terminal_read();

    if (key == TK_CLOSE || (key == TK_F4 && terminal_state(TK_ALT))) return Key::CLOSE;
    if (key == TK_MOUSE_SCROLL)
    {
        if (terminal_state(TK_MOUSE_WHEEL) < 0) return Key::MOUSE_SCROLL_UP;
        else return Key::MOUSE_SCROLL_DOWN;
    }

    switch(key)
    {
        case TK_RESIZED: return Key::RESIZED;
        case TK_ENTER: case TK_KP_ENTER: return Key::CR;
        case TK_BACKSPACE: case TK_DELETE: return Key::BACKSPACE;
        case TK_TAB: return Key::TAB;
        case TK_UP: return Key::ARROW_UP;
        case TK_DOWN: return Key::ARROW_DOWN;
        case TK_LEFT: return Key::ARROW_LEFT;
        case TK_RIGHT: return Key::ARROW_RIGHT;
        case TK_HOME: return Key::HOME;
        case TK_END: return Key::END;
        case TK_PAGEUP: return Key::PAGE_UP;
        case TK_PAGEDOWN: return Key::PAGE_DOWN;
        case TK_MOUSE_LEFT: return Key::MOUSE_LEFT;
        case TK_MOUSE_LEFT | TK_KEY_RELEASED: return Key::MOUSE_LEFT_RELEASED;
        case TK_MOUSE_MOVE: return Key::MOUSE_MOVED;
        default: return terminal_state(TK_CHAR);
    }
}

// Gets the X coordinate for the cell the mouse is pointing at.
int TerminalBLT::get_mouse_x() const { return terminal_state(TK_MOUSE_X); }

// Gets the X coordinate for the pixel the mouse is pointing at.
int TerminalBLT::get_mouse_x_pixel() const { return terminal_state(TK_MOUSE_PIXEL_X); }

// Gets the Y coordinate for the cell the mouse is pointing at.
int TerminalBLT::get_mouse_y() const { return terminal_state(TK_MOUSE_Y); }

// Gets the Y coordinate for the pixel the mouse is pointing at.
int TerminalBLT::get_mouse_y_pixel() const { return terminal_state(TK_MOUSE_PIXEL_Y); }

// Retrieves the size of the terminal (in cells, not pixels).
void TerminalBLT::get_size(int *w, int *h) const
{
    *w = terminal_state(TK_WIDTH);
    *h = terminal_state(TK_HEIGHT);
}

// Moves the cursor to the specified position.
void TerminalBLT::move_cursor(int x, int y)
{
    m_cursor_x = x;
    m_cursor_y = y;
}

// Prints a string at a given coordinate on the screen.
void TerminalBLT::print(std::string str, int x, int y, Colour col)
{
    if (!str.size()) return;
    if (col == Colour::WHITE_BG) set_background(Terminal::Colour::WHITE);
    
    if (str.find('[') != std::string::npos)
    {
        StrX::find_and_replace(str, "[", "[[");
        StrX::find_and_replace(str, "]", "]]");
    }

    if (str.find("{") == std::string::npos)
    {
        // If no colour codes are present, use the specified colour.
        const std::string chosen_colour = colour(col);
        str = "[color=" + chosen_colour + "]" + str;
    }

    while (str.find("{") != std::string::npos)
    {
        const bool mono = core()->tune()->monochrome_mode;
        if (StrX::find_and_replace(str, "{r}", mono ? "[color=g_grey]" : "[color=g_red_dark]")) continue;
        if (StrX::find_and_replace(str, "{R}", mono ? "[color=white]" : "[color=g_red]")) continue;
        if (StrX::find_and_replace(str, "{g}", mono ? "[color=g_grey]" : "[color=g_green_dark]")) continue;
        if (StrX::find_and_replace(str, "{G}", mono ? "[color=white]" : "[color=g_green]")) continue;
        if (StrX::find_and_replace(str, "{y}", mono ? "[color=g_grey]" : "[color=g_yellow_dark]")) continue;
        if (StrX::find_and_replace(str, "{Y}", mono ? "[color=white]" : "[color=g_yellow]")) continue;
        if (StrX::find_and_replace(str, "{u}", mono ? "[color=g_grey]" : "[color=g_blue_dark]")) continue;
        if (StrX::find_and_replace(str, "{U}", mono ? "[color=white]" : "[color=g_blue]")) continue;
        if (StrX::find_and_replace(str, "{m}", mono ? "[color=g_grey]" : "[color=g_magenta_dark]")) continue;
        if (StrX::find_and_replace(str, "{M}", mono ? "[color=white]" : "[color=g_magenta]")) continue;
        if (StrX::find_and_replace(str, "{c}", mono ? "[color=g_grey]" : "[color=g_cyan_dark]")) continue;
        if (StrX::find_and_replace(str, "{C}", mono ? "[color=white]" : "[color=g_cyan]")) continue;
        if (StrX::find_and_replace(str, "{w}", "[color=g_grey]")) continue;
        if (StrX::find_and_replace(str, "{W}", "[color=g_white]")) continue;
        if (StrX::find_and_replace(str, "{b}", "[color=g_black]")) continue;
        if (StrX::find_and_replace(str, "{B}", "[color=g_grey_dark]")) continue;
        break;
    }

    if (str.size() < 2 || str[0] != '[' || str[1] != 'c') terminal_color(color_from_name(colour(col).c_str()));
    terminal_print(x, y, str.c_str());
}

// Prints a character at a given coordinate on the screen.
void TerminalBLT::put(unsigned int letter, int x, int y, Colour col)
{
    if (col == Colour::WHITE_BG) set_background(Terminal::Colour::WHITE);
    terminal_color(color_from_name(colour(col).c_str()));
    terminal_put(x, y, letter);
}

// Refreshes the screen with changes made.
void TerminalBLT::refresh()
{
    if (m_cursor_visible) put('_', m_cursor_x, m_cursor_y, Colour::WHITE_BOLD);
    terminal_refresh();
}

// Sets the text background colour.
void TerminalBLT::set_background(Colour col) { terminal_bkcolor(colour(col).c_str()); }

// Returns true if the terminal window has been closed.
bool TerminalBLT::wants_to_close() const { return (terminal_read() == TK_CLOSE); }
