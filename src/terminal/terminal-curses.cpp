// terminal/terminal-curses.cpp -- Terminal interface for PDCurses/NCurses. See terminal.h for a full description of the Terminal class.
// Copyright (c) 2020 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "terminal/terminal-curses.hpp"
#include "uni/uni-core.hpp"

#ifdef GREAVE_TARGET_WINDOWS
#include "3rdparty/PDCurses/curses.h"
#else
#include <curses.h>
#endif


// Constructor, sets up Curses.
TerminalCurses::TerminalCurses()
{
	const std::shared_ptr<Tune> tune = core()->tune();
#ifdef GREAVE_TARGET_WINDOWS
	core()->guru()->log("Setting up PDCurses.");
#else
	core()->guru()->log("Setting up NCurses.");
#endif
	initscr();
	noecho();
	keypad(stdscr, true);
	curs_set(0);
	start_color();
	if (!can_change_color()) tune->curses_custom_colours = false;
	if (tune->curses_custom_colours)
	{
		auto redefine_colour = [](int colour, std::string value) {
			short r = 255, g = 255, b = 255;
			decode_hex_colour(value, r, g, b);
			init_color(colour, r, g, b);
		};

		redefine_colour(CUSTOM_RED, tune->colour_red);
		redefine_colour(CUSTOM_GREEN, tune->colour_green);
		redefine_colour(CUSTOM_YELLOW, tune->colour_yellow);
		redefine_colour(CUSTOM_BLUE, tune->colour_blue);
		redefine_colour(CUSTOM_MAGENTA, tune->colour_magenta);
		redefine_colour(CUSTOM_CYAN, tune->colour_cyan);
		redefine_colour(CUSTOM_WHITE, tune->colour_white);
		redefine_colour(CUSTOM_ORANGE, tune->colour_orange);
		redefine_colour(CUSTOM_PURPLE, tune->colour_purple);
		redefine_colour(CUSTOM_GREY, tune->colour_grey);
	}
	init_pair(1, COLOR_BLACK, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_YELLOW, COLOR_BLACK);
	init_pair(5, COLOR_BLUE, COLOR_BLACK);
	init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(7, COLOR_CYAN, COLOR_BLACK);
	init_pair(8, COLOR_WHITE, COLOR_BLACK);
	init_pair(9, COLOR_BLACK, COLOR_WHITE);

	if (tune->curses_custom_colours)
	{
		init_pair(CUSTOM_RED, CUSTOM_RED, COLOR_BLACK);
		init_pair(CUSTOM_GREEN, CUSTOM_GREEN, COLOR_BLACK);
		init_pair(CUSTOM_YELLOW, CUSTOM_YELLOW, COLOR_BLACK);
		init_pair(CUSTOM_BLUE, CUSTOM_BLUE, COLOR_BLACK);
		init_pair(CUSTOM_MAGENTA, CUSTOM_MAGENTA, COLOR_BLACK);
		init_pair(CUSTOM_CYAN, CUSTOM_CYAN, COLOR_BLACK);
		init_pair(CUSTOM_WHITE, CUSTOM_WHITE, COLOR_BLACK);
		init_pair(CUSTOM_ORANGE, CUSTOM_ORANGE, COLOR_BLACK);
		init_pair(CUSTOM_PURPLE, CUSTOM_PURPLE, COLOR_BLACK);
		init_pair(CUSTOM_GREY, CUSTOM_GREY, COLOR_BLACK);
		init_pair(CUSTOM_WHITE_BG, COLOR_BLACK, CUSTOM_WHITE);
	}

#ifdef GREAVE_TARGET_WINDOWS
	std::string ver_str = GreaveCore::GAME_VERSION;
#ifndef GREAVE_RELEASE
	ver_str += "D";
#endif
	PDC_set_title(("Greave " + ver_str).c_str());
#endif
}

// Destructor, cleans up Curses.
TerminalCurses::~TerminalCurses()
{
	echo();
	curs_set(1);
	endwin();
}

// Returns a colour pair code.
unsigned long TerminalCurses::colour(Colour col)
{
	const std::shared_ptr<Tune> tune = core()->tune();
	if (tune->monochrome_mode) switch(col)
	{
		case Colour::BLACK: return COLOR_PAIR(1);
		case Colour::BLACK_BOLD: return COLOR_PAIR(1) | A_BOLD;
		case Colour::RED: case Colour::GREEN: case Colour::YELLOW: case Colour::BLUE: case Colour::MAGENTA: case Colour::CYAN: case Colour::WHITE: return (tune->curses_custom_colours ? COLOR_PAIR(CUSTOM_GREY) : COLOR_PAIR(8));
		case Colour::RED_BOLD: case Colour::GREEN_BOLD: case Colour::YELLOW_BOLD: case Colour::BLUE_BOLD: case Colour::MAGENTA_BOLD: case Colour::CYAN_BOLD: case Colour::WHITE_BOLD:
			return (tune->curses_custom_colours ? COLOR_PAIR(CUSTOM_WHITE) :COLOR_PAIR(8) | A_BOLD);
		case Colour::WHITE_BG: return (tune->curses_custom_colours ? COLOR_PAIR(CUSTOM_WHITE_BG) : COLOR_PAIR(9) | A_BOLD);
		default: return 0;	// This should be impossible, but keeps the compiler happy.
	}
	else switch(col)
	{
		case Colour::BLACK: return COLOR_PAIR(1);
		case Colour::BLACK_BOLD: return COLOR_PAIR(1) | A_BOLD;
		case Colour::RED: return COLOR_PAIR(2);
		case Colour::RED_BOLD: return (tune->curses_custom_colours ? COLOR_PAIR(CUSTOM_RED) : COLOR_PAIR(2) | A_BOLD);
		case Colour::GREEN: return COLOR_PAIR(3);
		case Colour::GREEN_BOLD: return (tune->curses_custom_colours ? COLOR_PAIR(CUSTOM_GREEN) : COLOR_PAIR(3) | A_BOLD);
		case Colour::YELLOW: return (tune->curses_custom_colours ? COLOR_PAIR(CUSTOM_ORANGE) : COLOR_PAIR(4));
		case Colour::YELLOW_BOLD: return (tune->curses_custom_colours ? COLOR_PAIR(CUSTOM_YELLOW) : COLOR_PAIR(4) | A_BOLD);
		case Colour::BLUE: return COLOR_PAIR(5);
		case Colour::BLUE_BOLD: return (tune->curses_custom_colours ? COLOR_PAIR(CUSTOM_BLUE) : COLOR_PAIR(5) | A_BOLD);
		case Colour::MAGENTA: return (tune->curses_custom_colours ? COLOR_PAIR(CUSTOM_PURPLE) : COLOR_PAIR(6));
		case Colour::MAGENTA_BOLD: return (tune->curses_custom_colours ? COLOR_PAIR(CUSTOM_MAGENTA) : COLOR_PAIR(6) | A_BOLD);
		case Colour::CYAN: return COLOR_PAIR(7);
		case Colour::CYAN_BOLD: return (tune->curses_custom_colours ? COLOR_PAIR(CUSTOM_CYAN) : COLOR_PAIR(7) | A_BOLD);
		case Colour::WHITE: return (tune->curses_custom_colours ? COLOR_PAIR(CUSTOM_GREY) : COLOR_PAIR(8));
		case Colour::WHITE_BOLD: return (tune->curses_custom_colours ? COLOR_PAIR(CUSTOM_WHITE) : COLOR_PAIR(8) | A_BOLD);
		case Colour::WHITE_BG: return (tune->curses_custom_colours ? COLOR_PAIR(CUSTOM_WHITE_BG) : COLOR_PAIR(9));
		default: return 0;	// This should be impossible, but keeps the compiler happy.
	}
}

// Clears the screen.
void TerminalCurses::cls() { clear(); }

// Makes the cursor visible or invisible.
void TerminalCurses::cursor(bool visible) { curs_set(visible ? 1 : 0); }

// Decodes a hex-code colour into RGB values.
void TerminalCurses::decode_hex_colour(const std::string &col, short &r, short &g, short &b)
{
	if (col.size() != 6) return;
	r = Util::htoi(col.substr(0, 2)) * 3.92f;
	g = Util::htoi(col.substr(2, 2)) * 3.92f;
	b = Util::htoi(col.substr(4, 2)) * 3.92f;
}

// Fills a given area in with the specified colour. Currently nonfunctional on Curses.
void TerminalCurses::fill(int, int, int, int, Colour) { }

// Gets keyboard input from the terminal.
int TerminalCurses::get_key()
{
	int key = getch();
	switch(key)
	{
		case 3: case 4: return Key::CLOSE;		// Ctrl-C or Ctrl-D close the console window.
		case KEY_RESIZE: return Key::RESIZED;	// Window resized event.
		case KEY_UP: return Key::ARROW_UP;
		case KEY_DOWN: return Key::ARROW_DOWN;
		case KEY_LEFT: return Key::ARROW_LEFT;
		case KEY_RIGHT: return Key::ARROW_RIGHT;
		case KEY_HOME: return Key::HOME;
		case KEY_END: return Key::END;
		case KEY_PPAGE: return Key::PAGE_UP;
		case KEY_NPAGE: return Key::PAGE_DOWN;
		case KEY_BACKSPACE: return Key::BACKSPACE;
	}

	if (key > 255 || key < 0) return -1;	// Any other unrecognized keys are just returned as -1.
	return key;
}

// Retrieves the size of the terminal (in cells, not pixels).
void TerminalCurses::get_size(int *w, int *h)
{
	*w = getmaxx(stdscr);
	*h = getmaxy(stdscr);
}

// Moves the cursor to the specified position.
void TerminalCurses::move_cursor(int x, int y) { move(y, x); }

// Prints a string at a given coordinate on the screen.
void TerminalCurses::print(std::string str, int x, int y, Colour col)
{
	if (!str.size()) return;
	if (str.find("{") == std::string::npos)
	{
		// If no colour codes are present, this is fairly easy.
		const unsigned long ansi_code = colour(col);
		attron(ansi_code);
		mvprintw(y, x, str.c_str());
		attroff(ansi_code);
		return;
	}

	// Colour codes need to be parsed. Curses doesn't support multiple colour codes in a single printw(), so we're gonna have to get creative.

	if (str[0] != '{') str = "{w}" + str;	// If there isn't a colour tag specified at the start, start it with white.

	while (str.size())
	{
		std::string first_word;
		size_t tag_pos = str.substr(1).find_first_of('{');	// We skip ahead one char, to ignore the opening brace. We want to find the *next* opening brace.
		if (tag_pos != std::string::npos)
		{
			first_word = str.substr(0, tag_pos + 1);
			str = str.substr(tag_pos + 1);
		}
		else
		{
			first_word = str;
			str = "";
		}

		while (first_word.size() >= 3 && first_word[0] == '{' && first_word[2] == '}')
		{
			const std::string tag = first_word.substr(0, 3);
			first_word = first_word.substr(3);
			switch(tag[1])
			{
				case 'r': col = Colour::RED_BOLD; break;
				case 'g': col = Colour::GREEN_BOLD; break;
				case 'y': col = Colour::YELLOW_BOLD; break;
				case 'b': col = Colour::BLUE_BOLD; break;
				case 'm': col = Colour::MAGENTA_BOLD; break;
				case 'c': col = Colour::CYAN_BOLD; break;
				case 'w': col = Colour::WHITE_BOLD; break;
				case 'o': col = Colour::YELLOW; break;
				case 'p': col = Colour::MAGENTA; break;
				case 'e': col = Colour::WHITE; break;
			}
		}

		const unsigned long ansi_code = colour(col);
		attron(ansi_code);
		const unsigned int first_word_size = first_word.size();
		Util::find_and_replace(first_word, "%", "%%");
		mvprintw(y, x, first_word.c_str());
		attroff(ansi_code);
		x += first_word_size;
	}
}

// Prints a character at a given coordinate on the screen.
void TerminalCurses::put(unsigned int letter, int x, int y, Colour col)
{
	if (letter > 255) letter = '?';
	const unsigned long ansi_code = colour(col);
	attron(ansi_code);
	mvaddch(y, x, letter);
	attroff(ansi_code);
}

// Refreshes the screen with changes made.
void TerminalCurses::refresh() { ::refresh(); }

// Sets the text background colour. Currently nonfunctional on Curses.
void TerminalCurses::set_background(Colour) { }

// Returns true if the player uses Ctrl-C, Ctrl-D or escape.
bool TerminalCurses::wants_to_close()
{
	const char ch = getch();
	return (ch == 3 || ch == 4 || ch == 27);
}
