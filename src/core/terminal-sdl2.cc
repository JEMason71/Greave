// core/terminal-sdl2.cc -- Terminal interface for SDL2/SDL_ttf. See core/terminal.hpp for a full description of the Terminal class.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifdef GREAVE_INCLUDE_SDL

#include "3rdparty/LodePNG/bmp2png.h"
#ifdef GREAVE_TOLK
#include "3rdparty/Tolk/Tolk.h"
#endif
#include "core/core.h"
#include "core/filex.h"
#include "core/strx.h"
#include "core/terminal-sdl2.h"

#include <ctime>
#include <thread>


// Constructor, sets up SDL2.
TerminalSDL2::TerminalSDL2() : cursor_visible_(false), cursor_x_(0), cursor_y_(0), font_(nullptr), init_sdl_(false), init_sdl_ttf_(false), mouse_x_(0), mouse_y_(0), renderer_(nullptr), screenshot_msg_time_(0), screenshot_taken_(0), window_(nullptr)
{
    const std::shared_ptr<Prefs> prefs = core()->prefs();
    if (SDL_Init(SDL_INIT_VIDEO) < 0) throw std::runtime_error("Could not initialize SDL: " + std::string(SDL_GetError()));
    else init_sdl_ = true;
    if (TTF_Init() < 0) throw std::runtime_error("Could not initialize SDL_ttf: " + std::string(TTF_GetError()));
    else init_sdl_ttf_ = true;
    SDL_StartTextInput();

    // Load the font!
    font_ = TTF_OpenFont(("data/fonts/" + prefs->sdl_font).c_str(), prefs->sdl_font_size);
    if (!font_) throw std::runtime_error("Could not load TTF font!");

    // Determine the size, in pixels, of the chosen font. I very much hope this is fixed-width, or weird shit is gonna happen here.
    SDL_Surface *temp_surf = TTF_RenderText_Solid(font_, "#", {255, 255, 255, 255});
    font_width_ = temp_surf->w;
    font_height_ = temp_surf->h;
    SDL_FreeSurface(temp_surf);

    // Determine the size of the main window.
    const std::vector<std::string> size_exp = StrX::string_explode(prefs->sdl_console_size, "x");
    if (size_exp.size() != 2) throw std::runtime_error("Invalid SDL console size specified in prefs.yml!");
    window_w_ = std::stol(size_exp.at(0)) * font_width_;
    window_h_ = std::stol(size_exp.at(1)) * font_height_;

    // Create the main window!
    window_ = SDL_CreateWindow(("Greave " + std::string(CoreConstants::GAME_VERSION)).c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_w_, window_h_,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window_) throw std::runtime_error("Could not create SDL window: " + std::string(SDL_GetError()));

    // Create the renderer for the window.
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | (prefs->sdl_vsync ? SDL_RENDERER_PRESENTVSYNC : 0));
    if (!renderer_) throw std::runtime_error("Could not create SDL renderer: " + std::string(SDL_GetError()));

    // Set the colours up.
    init_colours();
}

// Destructor, cleans up SDL2.
TerminalSDL2::~TerminalSDL2()
{
    if (font_)
    {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
    if (renderer_)
    {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_)
    {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    if (init_sdl_ttf_) TTF_Quit();
    if (init_sdl_) SDL_Quit();
}

// Returns the height of a single cell, in pixels.
int TerminalSDL2::cell_height() const { return font_height_; }

// Clears the screen.
void TerminalSDL2::cls()
{
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0xFF);
    SDL_RenderClear(renderer_);
}

// Converts a colour code into a more useful form.
void TerminalSDL2::colour_to_rgb(Colour col, uint8_t *r, uint8_t *g, uint8_t *b) const
{
    const auto it = colour_map_.find(col);
    if (it == colour_map_.end()) throw std::runtime_error("Invalid colour!");
    *r = it->second.r;
    *g = it->second.g;
    *b = it->second.b;
}

// Makes the cursor visible or invisible.
void TerminalSDL2::cursor(bool visible) { cursor_visible_ = visible; }

// Fills a given area in with the specified colour.
void TerminalSDL2::fill(int x, int y, int w, int h, Colour col)
{
    uint8_t r, g, b;
    colour_to_rgb(col, &r, &g, &b);
    SDL_SetRenderDrawColor(renderer_, r, g, b, 0xFF);
    SDL_Rect rect = { x * font_width_, y * font_height_, w * font_width_, h * font_height_ };
    SDL_RenderFillRect(renderer_, &rect);
}

// Gets keyboard input from the terminal.
int TerminalSDL2::get_key()
{
    SDL_Event event;
    while (true)
    {
        SDL_WaitEvent(&event);
        switch (event.type)
        {
            case SDL_QUIT: return Key::CLOSE;
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                    case SDL_WINDOWEVENT_CLOSE: return Key::CLOSE;
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        window_w_ = event.window.data1;
                        window_h_ = event.window.data2;
                        refresh();
                        return Key::RESIZED;
                    case SDL_WINDOWEVENT_EXPOSED: refresh(); break;
                }
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_BACKSPACE: case SDLK_KP_BACKSPACE: return Key::BACKSPACE;
                    case SDLK_TAB: case SDLK_KP_TAB: return Key::TAB;
                    case SDLK_RETURN: case SDLK_RETURN2: case SDLK_KP_ENTER: return Key::CR;
                    case SDLK_UP: case SDLK_KP_8: return Key::ARROW_UP;
                    case SDLK_DOWN: case SDLK_KP_2: return Key::ARROW_DOWN;
                    case SDLK_LEFT: case SDLK_KP_4: return Key::ARROW_LEFT;
                    case SDLK_RIGHT: case SDLK_KP_6: return Key::ARROW_RIGHT;
                    case SDLK_HOME: case SDLK_KP_7: return Key::HOME;
                    case SDLK_END: case SDLK_KP_1: return Key::END;
                    case SDLK_PAGEUP: case SDLK_KP_9: return Key::PAGE_UP;
                    case SDLK_PAGEDOWN: case SDLK_KP_3: return Key::PAGE_DOWN;
                    case SDLK_PRINTSCREEN: screenshot(); return Key::RESIZED;
#ifdef GREAVE_TOLK
                    case SDLK_ESCAPE: Tolk_Silence(); break;
#endif
                }
                break;
            case SDL_TEXTINPUT:
                return event.text.text[0];
            case SDL_MOUSEWHEEL:
                if (event.wheel.y > 0) return Key::MOUSE_SCROLL_UP;
                else if (event.wheel.y < 0) return Key::MOUSE_SCROLL_DOWN;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) return Key::MOUSE_LEFT;
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) return Key::MOUSE_LEFT_RELEASED;
                break;
            case SDL_MOUSEMOTION:
                mouse_x_ = event.motion.x;
                mouse_y_ = event.motion.y;
                return Key::MOUSE_HAS_MOVED;
        }
    }
}

// Gets the X coordinate for the cell the mouse is pointing at.
int TerminalSDL2::get_mouse_x() const { return mouse_x_ / font_width_; }

// Gets the X coordinate for the pixel the mouse is pointing at.
int TerminalSDL2::get_mouse_x_pixel() const { return mouse_x_; }

// Gets the Y coordinate for the cell the mouse is pointing at.
int TerminalSDL2::get_mouse_y() const { return mouse_y_ / font_height_; }

// Gets the Y coordinate for the pixel the mouse is pointing at.
int TerminalSDL2::get_mouse_y_pixel() const { return mouse_y_; }

// Retrieves the size of the terminal (in cells, not pixels).
void TerminalSDL2::get_size(int *w, int *h) const
{
    *w = window_w_ / font_width_;
    *h = window_h_ / font_height_;
}

// Loads the colours from prefs.yml into RGB values.
void TerminalSDL2::init_colours()
{
    const std::shared_ptr<Prefs> prefs = core()->prefs();

    auto populate_colour_map = [this](const std::string &colour_str, Colour colour_enum)
    {

        if (colour_str.size() != 6) throw std::runtime_error("Invalid colour value: " + colour_str);
        uint8_t r = StrX::htoi(colour_str.substr(0, 2));
        uint8_t g = StrX::htoi(colour_str.substr(2, 2));
        uint8_t b = StrX::htoi(colour_str.substr(4, 2));
        colour_map_.insert(std::pair<Colour, RGB>(colour_enum, {r,g,b}));
    };

    populate_colour_map(prefs->colour_black, Colour::BLACK);
    populate_colour_map(prefs->colour_blue, Colour::BLUE_BOLD);
    populate_colour_map(prefs->colour_blue_dark, Colour::BLUE);
    populate_colour_map(prefs->colour_cyan, Colour::CYAN_BOLD);
    populate_colour_map(prefs->colour_cyan_dark, Colour::CYAN);
    populate_colour_map(prefs->colour_green, Colour::GREEN_BOLD);
    populate_colour_map(prefs->colour_green_dark, Colour::GREEN);
    populate_colour_map(prefs->colour_grey, Colour::WHITE);
    populate_colour_map(prefs->colour_grey_dark, Colour::BLACK_BOLD);
    populate_colour_map(prefs->colour_grey_darkest, Colour::DARKEST_GREY);
    populate_colour_map(prefs->colour_magenta, Colour::MAGENTA_BOLD);
    populate_colour_map(prefs->colour_magenta_dark, Colour::MAGENTA);
    populate_colour_map(prefs->colour_red, Colour::RED_BOLD);
    populate_colour_map(prefs->colour_red_dark, Colour::RED);
    populate_colour_map(prefs->colour_white, Colour::WHITE_BOLD);
    populate_colour_map(prefs->colour_yellow, Colour::YELLOW_BOLD);
    populate_colour_map(prefs->colour_yellow_dark, Colour::YELLOW);
    populate_colour_map(prefs->colour_white, Colour::WHITE_BG);
}

// Moves the cursor to the specified position.
void TerminalSDL2::move_cursor(int x, int y)
{
    cursor_x_ = x;
    cursor_y_ = y;
}

// Internal rendering code, after print() has parsed the colour tags.
void TerminalSDL2::print_internal(std::string str, int x, int y, Colour col)
{
    if (col == Colour::WHITE_BG)
    {
        fill(x, y, 1, 1, Colour::WHITE);
        col = Colour::WHITE;
    }

    uint8_t r, g, b;
    colour_to_rgb(col, &r, &g, &b);
    SDL_Surface* font_surf = TTF_RenderText_Blended(font_, str.c_str(), {r, g, b, 255});
    SDL_Texture* font_tex = SDL_CreateTextureFromSurface(renderer_, font_surf);
    SDL_Rect rect = { x * font_width_, y * font_height_, static_cast<int>(str.size() * font_width_), font_height_ };
    SDL_RenderCopy(renderer_, font_tex, nullptr, &rect);
    SDL_DestroyTexture(font_tex);
    SDL_FreeSurface(font_surf);
}

// Prints a character at a given coordinate on the screen.
void TerminalSDL2::put(uint16_t letter, int x, int y, Colour col) { print(std::string(1, static_cast<char>(letter)), x, y, col); }

// Refreshes the screen with changes made.
void TerminalSDL2::refresh()
{
    if (cursor_visible_)
    {
        fill(cursor_x_, cursor_y_, 1, 1, Colour::DARKEST_GREY);
        put('_', cursor_x_, cursor_y_, Colour::WHITE_BOLD);
    }

    if (screenshot_taken_)
    {
        if (screenshot_msg_time_ > std::time(nullptr))
        {
            const std::string sshot_text = "Screenshot taken: greave" + std::to_string(screenshot_taken_) + ".png";
            fill(0, 0, sshot_text.size(), 1, Colour::BLACK);
            print(sshot_text, 0, 0, Colour::GREEN_BOLD);
        }
        else screenshot_taken_ = 0;
    }

    SDL_RenderPresent(renderer_);
}

// Takes a screenshot!
void TerminalSDL2::screenshot()
{
    FileX::make_dir("userdata/screenshots");

    // Determine the filename for the screenshot.
    int sshot = 0;
    std::string filename;
    while(true)
    {
        filename = "userdata/screenshots/greave" + std::to_string(++sshot);
        if (!(FileX::file_exists(filename + ".png") || FileX::file_exists(filename + ".bmp") || FileX::file_exists(filename + ".tmp"))) break;
        if (sshot > 1000000) return;    // Just give up if we have an absurd amount of files.
    }

    // Take a screenshot in BMP format (SDL_image can do PNG exports directly, but that's a lot of extra overhead). Using LodePNG is way more lightweight.
    SDL_Surface* temp_surf = SDL_CreateRGBSurface(0, window_w_, window_h_, 32, 0, 0, 0, 0);
    SDL_RenderReadPixels(renderer_, nullptr, temp_surf->format->format, temp_surf->pixels, temp_surf->pitch);
    SDL_SaveBMP(temp_surf, (filename + ".tmp").c_str());
    SDL_FreeSurface(temp_surf);

    // Convert the BMP screenshot to PNG format.
    std::thread(convert_png, filename).detach();

    // Display the screenshot taken message.
    screenshot_taken_ = sshot;
    screenshot_msg_time_ = std::time(nullptr) + 2;
    refresh();
}

// Returns true if the player has tried to close the SDL window.
bool TerminalSDL2::wants_to_close() const { return false; }
#endif  // GREAVE_INCLUDE_SDL
