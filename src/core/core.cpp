// core/core.cpp -- Main program entry, initialization and cleanup routines, and the core game loop.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "actions/look.hpp" // temp
#include "core/core.hpp"
#include "core/filex.hpp"
#include "core/guru.hpp"
#include "core/message.hpp"
#include "core/parser.hpp"
#include "core/strx.hpp"
#include "core/terminal-blt.hpp"
#include "core/terminal-curses.hpp"
#include "core/tune.hpp"
#include "world/mobile.hpp"
#include "world/room.hpp"
#include "world/world.hpp"

#include <thread>

#ifdef GREAVE_TARGET_WINDOWS
#include <windows.h>
#endif

#ifdef GREAVE_TOLK
#include "3rdparty/Tolk/Tolk.h"
#include <regex>
#endif


std::shared_ptr<Core> greave = nullptr;   // The main Core object.

const std::string   Core::GAME_VERSION =        "pre-alpha";    // The game's version number.
const unsigned int  Core::MSG_FLAG_INTERRUPT =  1;              // Flags for the message() function.
const unsigned int  Core::SAVE_VERSION =        1;              // The version number for saved game files. This should increment when old saves can no longer be loaded.
const unsigned int  Core::TAGS_PERMANENT =      10000;          // The tag number at which tags are considered permanent.


// Main program entry point.
int main(int argc, char* argv[])
{
    // Check command-line parameters.
    std::vector<std::string> parameters(argv, argv + argc);

    greave = std::make_shared<Core>();
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
Core::Core() : m_message_log(nullptr), m_parser(nullptr), m_save_slot(0), m_sql_unique_id(0), m_terminal(nullptr), m_tune(nullptr), m_world(nullptr) { }

// Cleans up after we're d one.
void Core::cleanup()
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
const std::shared_ptr<Guru> Core::guru() const
{
    if (!m_guru_meditation) exit(EXIT_FAILURE);
    return m_guru_meditation;
}

// Sets up the core game classes and data.
void Core::init()
{
    FileX::make_dir("userdata");
    FileX::make_dir("userdata/save");

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

    std::string terminal_choice = StrX::str_tolower(m_tune->terminal);
#ifdef GREAVE_TARGET_WINDOWS
    if (terminal_choice != "curses") FreeConsole();
#endif

    // Set up our terminal emulator.
    if (terminal_choice == "blt") m_terminal = std::make_shared<TerminalBLT>();
    else if (terminal_choice == "curses") m_terminal = std::make_shared<TerminalCurses>();
    else m_guru_meditation->halt("Invalid terminal specified in tune.yml");

    // Sets up the main message log window.
    m_message_log = std::make_shared<MessageLog>();

    // Tell the Guru system we're finished setting up the terminal and message window.
    guru()->console_ready();

    // Sets up the text parser.
    m_parser = std::make_shared<Parser>();
}

// Loads a specified slot's saved game.
void Core::load(unsigned int save_slot)
{
    const std::string save_filename = "userdata/save/save-" + std::to_string(save_slot) + ".sqlite";
    m_save_slot = save_slot;
    std::shared_ptr<SQLite::Database> save_db = std::make_shared<SQLite::Database>(save_filename, SQLite::OPEN_READONLY);

    SQLite::Statement version_query(*save_db, "PRAGMA user_version");
    if (version_query.executeStep())
    {
        const unsigned int version = version_query.getColumn(0).getInt();
        if (version != Core::SAVE_VERSION) throw std::runtime_error("Invalid saved game version!");
    }
    else throw std::runtime_error("Could not load version information from save file!");

    m_world->load(save_db);
}

// The main game loop.
void Core::main_loop()
{
    // bröther may I have some lööps
    while (true)
    {
        const std::string input = m_message_log->render_message_log();
        m_parser->parse(input);
    }
}

// Prints a message in the message log.
void Core::message(std::string msg, uint32_t flags)
{
    m_message_log->msg(msg);

#ifdef GREAVE_TOLK
    const bool interrupt = ((flags & Core::MSG_FLAG_INTERRUPT) == Core::MSG_FLAG_INTERRUPT);
    if (m_tune->screen_reader_external || m_tune->screen_reader_sapi)
    {
        if (m_tune->screen_reader_process_square_brackets)
        {
            StrX::find_and_replace(msg, "[", "(");
            StrX::find_and_replace(msg, "]", ").");
        }
        std::regex filter("\\{.*?\\}");
        std::string msg_voice = std::regex_replace(msg, filter, "");
        if (msg_voice.size() >= 2 && msg_voice[0] == '>') msg_voice = msg_voice.substr(2);
        const std::wstring msg_wide(msg_voice.begin(), msg_voice.end());
        Tolk_Output(msg_wide.c_str(), interrupt);
        if (interrupt) m_message_log->clear_latest_messages();
        m_message_log->add_latest_message(msg_voice);
    }
#endif
}

// Returns a pointer to the MessageLog object.
const std::shared_ptr<MessageLog> Core::messagelog() const { return m_message_log; }

// Starts the game.
void Core::play()
{
    m_world = std::make_shared<World>();

    if (FileX::file_exists("userdata/save/save-0.sqlite")) load(0);
    else
    {
        m_world->player()->set_location("TEST_ROOM");
        ActionLook::look(m_world->player());
    }

    main_loop();
}

// Saves the game to disk.
void Core::save()
{
    const std::string save_filename_base = "userdata/save/save-" + std::to_string(m_save_slot);
    const std::string save_filename = save_filename_base + ".sqlite";
    const std::string save_filename_old = save_filename_base + ".old";
    if (FileX::file_exists(save_filename_old)) FileX::delete_file(save_filename_old);
    if (FileX::file_exists(save_filename)) FileX::rename_file(save_filename, save_filename_old);

    std::shared_ptr<SQLite::Database> save_db = std::make_shared<SQLite::Database>(save_filename, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    save_db->exec("PRAGMA user_version = " + std::to_string(SAVE_VERSION));
    m_sql_unique_id = 0;    // We're making a new save file each time, so we can reset the unique ID counter.

    SQLite::Transaction transaction(*save_db);
    m_world->save(save_db);
    transaction.commit();

    message("{M}Game saved in slot {Y}" + std::to_string(m_save_slot) + "{M}.");
}

// Retrieves a new unique SQL ID.
uint32_t Core::sql_unique_id() { return ++m_sql_unique_id; }

// Returns a pointer  to the terminal emulator object.
const std::shared_ptr<Terminal> Core::terminal() const { return m_terminal; }

// Returns a pointer to the Tune object.
const std::shared_ptr<Tune> Core::tune() const { return m_tune; }

// Returns a pointer to the World object.
const std::shared_ptr<World> Core::world() const { return m_world; }

// Allows external access to the Core object.
const std::shared_ptr<Core> core()
{
    if (!greave) exit(EXIT_FAILURE);
    else return greave;
}
