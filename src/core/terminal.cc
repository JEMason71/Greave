// core/terminal.cc -- Middleware layer between the game proper and the terminal emulator being used (Curses, SDL, etc.)
// This way, multiple alternative terminal emulators can be plugged in without affecting the rest of the code.
// The base Terminal class is mostly virtual; derived classes should handle code specific to their specific terminal emulator.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "core/terminal.h"


namespace greave {

// Prints a string at a given coordinate on the screen. This particular function actually parses the colour strings, then calls print_internal().
void Terminal::print(std::string str, int x, int y, Colour col)
{
    if (!str.size()) return;

    // Convert invisible space characters (`) into actual spaces on the screen.
    // This is so that other string-formatting functions treat ` as characters, but they're rendered as spaces.
    size_t nbsp_pos;
    while ((nbsp_pos = str.find("`")) != std::string::npos)
        str.at(nbsp_pos) = ' ';

    if (str.find("{") == std::string::npos)
    {
        // No colour codes/ That makes things easy!
        print_internal(str, x, y, col);
        return;
    }

    while (str.size())
    {
        std::string first_word;
        size_t tag_pos = str.substr(1).find_first_of('{');  // We skip ahead one char, to ignore the opening brace. We want to find the *next* opening brace.
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
                case 'b': col = Colour::BLACK; break;
                case 'B': col = Colour::BLACK_BOLD; break;
                case 'r': col = Colour::RED; break;
                case 'R': col = Colour::RED_BOLD; break;
                case 'g': col = Colour::GREEN; break;
                case 'G': col = Colour::GREEN_BOLD; break;
                case 'y': col = Colour::YELLOW; break;
                case 'Y': col = Colour::YELLOW_BOLD; break;
                case 'u': col = Colour::BLUE; break;
                case 'U': col = Colour::BLUE_BOLD; break;
                case 'm': col = Colour::MAGENTA; break;
                case 'M': col = Colour::MAGENTA_BOLD; break;
                case 'c': col = Colour::CYAN; break;
                case 'C': col = Colour::CYAN_BOLD; break;
                case 'w': col = Colour::WHITE; break;
                case 'W': col = Colour::WHITE_BOLD; break;
            }
        }

        const size_t first_word_size = first_word.size();
        print_internal(first_word, x, y, col);
        x += first_word_size;
    }
}

}   // namespace greave
