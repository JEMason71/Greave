// core/core.cpp -- Main program entry, initialization and cleanup routines, and the core game loop.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/Tolk/Tolk.h"
#include "core/core.hpp"
#include "core/guru.hpp"
#include "core/message.hpp"
#include "core/tune.hpp"
#include "core/utility.hpp"
#include "terminal/terminal-blt.hpp"
#include "terminal/terminal-curses.hpp"
#include "world/room.hpp"
#include "world/world.hpp"

#include <thread>
#ifdef GREAVE_TARGET_WINDOWS
#include <windows.h>
#endif
#ifdef GREAVE_TOLK
#include <regex>
#endif


std::shared_ptr<GreaveCore> greave = nullptr;   // The main GreaveCore object.

const std::string   GreaveCore::GAME_VERSION =    "pre-alpha";  // The game's version number.
const unsigned int  GreaveCore::MSG_FLAG_INTERRUPT = 1; // Flags for the message() function.


// Main program entry point.
int main(int argc, char* argv[])
{
    // Check command-line parameters.
    std::vector<std::string> parameters(argv, argv + argc);

    greave = std::make_shared<GreaveCore>();
    try
    {
        greave->init();
        greave->play();
        greave->cleanup();
    }
    catch (std::exception& e)
    {
        greave->guru()->halt(e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// Constructor, doesn't do too much aside from setting default values for member variables. Use init() to set things up.
GreaveCore::GreaveCore() : m_message_log(nullptr), m_terminal(nullptr), m_tune(nullptr), m_world(nullptr) { }

// Cleans up after we're d one.
void GreaveCore::cleanup()
{
    // Tell Guru to revert to exit() if an error happens at this point.
    guru()->console_ready(false);

#ifdef GREAVE_TOLK
    // Clean up Tolk, if we're on Windows.
    if (m_tune->screen_reader_external || m_tune->screen_reader_sapi) Tolk_Unload();
#endif

    m_terminal = nullptr;   // It's a smart pointer, so this'll run the destructor code.
}

// Returns a pointer to the Guru Meditation object.
const std::shared_ptr<Guru> GreaveCore::guru() const
{
    if (!m_guru_meditation) exit(EXIT_FAILURE);
    return m_guru_meditation;
}

// Sets up the core game classes and data.
void GreaveCore::init()
{
    Util::make_dir("userdata");

    // Sets up the error-handling subsystem.
    m_guru_meditation = std::make_shared<Guru>("userdata/log.txt");

    // Set up the tune settings.
    m_tune = std::make_shared<Tune>();

#ifdef GREAVE_TOLK
    // Set up Tolk if we're on Windows.
    if (m_tune->screen_reader_sapi) Tolk_TrySAPI(true); // Enable SAPI.
    if (m_tune->screen_reader_external || m_tune->screen_reader_sapi) Tolk_Load();
    if (Tolk_DetectScreenReader())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Tolk_Silence();
    }
#endif

    std::string terminal_choice = Util::str_tolower(m_tune->terminal);
#ifdef GREAVE_TARGET_WINDOWS
    if (terminal_choice != "curses") FreeConsole();
#endif

    // Set up our terminal emulator.
    if (terminal_choice == "blt") m_terminal = std::make_shared<TerminalBLT>();
    else if (terminal_choice == "curses") m_terminal = std::make_shared<TerminalCurses>();
    else m_guru_meditation->halt("Invalid terminal specified in tune.yml");

    // Sets up the main message log window.
    m_message_log = std::make_shared<MessageLog>();

    // Tell the Guru system we're finished setting up.
    guru()->console_ready();

    // Load the areas from YAML data.
    Room::load_room_pool();
}

// Prints a message in the message log.
void GreaveCore::message(std::string msg, uint32_t flags)
{
    m_message_log->msg(msg);

#ifdef GREAVE_TOLK
    const bool interrupt = ((flags & GreaveCore::MSG_FLAG_INTERRUPT) == GreaveCore::MSG_FLAG_INTERRUPT);
    if (m_tune->screen_reader_external || m_tune->screen_reader_sapi)
    {
        if (m_tune->screen_reader_process_square_brackets)
        {
            Util::find_and_replace(msg, "[", "(");
            Util::find_and_replace(msg, "]", ").");
        }
        std::regex filter("\\{.*?\\}");
        std::string msg_voice = std::regex_replace(msg, filter, "");
        if (msg_voice.size() >= 2 && msg_voice[0] == '>') msg_voice = msg_voice.substr(2);
        const std::wstring msg_wide(msg_voice.begin(), msg_voice.end());
        Tolk_Output(msg_wide.c_str(), interrupt);
        if (interrupt) m_message_log->m_latest_messages.clear();
        m_message_log->m_latest_messages.push_back(msg_voice);
    }
#endif
}

// Returns a pointer to the MessageLog object.
const std::shared_ptr<MessageLog> GreaveCore::messagelog() const { return m_message_log; }

// Starts the game.
void GreaveCore::play()
{
    m_world = std::make_shared<World>();
    m_world->main_loop();
}

// Returns a pointer  to the terminal emulator object.
const std::shared_ptr<Terminal> GreaveCore::terminal() const { return m_terminal; }

// Returns a pointer to the Tune object.
const std::shared_ptr<Tune> GreaveCore::tune() const { return m_tune; }

// Returns a pointer to the World object.
const std::shared_ptr<World> GreaveCore::world() const { return m_world; }

// Allows external access to the GreaveCore object.
const std::shared_ptr<GreaveCore> core()
{
    if (!greave) exit(EXIT_FAILURE);
    else return greave;
}
