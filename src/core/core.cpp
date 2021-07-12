// core/core.cpp -- Main program entry, initialization and cleanup routines, and the core game loop.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "actions/help.hpp"
#include "actions/look.hpp"
#include "core/bones.hpp"
#include "core/core.hpp"
#include "core/filex.hpp"
#include "core/guru.hpp"
#include "core/list.hpp"
#include "core/message.hpp"
#include "core/parser.hpp"
#include "core/prefs.hpp"
#include "core/random.hpp"
#include "core/strx.hpp"
#include "core/terminal-curses.hpp"
#include "core/terminal-sdl2.hpp"
#include "world/player.hpp"
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

const std::string   Core::GAME_VERSION =    "pre-alpha";    // The game's version number.
const uint32_t      Core::SAVE_VERSION =    63;             // The version number for saved game files. This should increment when old saves can no longer be loaded.
const uint16_t      Core::TAGS_PERMANENT =  10000;          // The tag number at which tags are considered permanent.


// Main program entry point.
int main(int argc, char* argv[])
{
    // Check command-line parameters.
    std::vector<std::string> parameters(argv, argv + argc);
    bool dry_run = false;
    if (parameters.size() >= 2)
        for (auto param : parameters)
            if (!param.compare("-dry-run")) dry_run = true;

    greave = std::make_shared<Core>();
    try
    {
        greave->init(dry_run);
        if (dry_run)
        {
            auto new_world =std::make_shared<World>();
        }
        else
        {
            greave->title();
            greave->main_loop();
        }
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
Core::Core() : m_message_log(nullptr), m_parser(nullptr), m_rng(nullptr), m_save_slot(0), m_sql_unique_id(0), m_terminal(nullptr), m_prefs(nullptr), m_world(nullptr) { }

// Cleans up after we're d one.
void Core::cleanup()
{
    // Tell Guru to revert to exit() if an error happens at this point.
    guru()->console_ready(false);

#ifdef GREAVE_TOLK
    // Clean up Tolk, if we're on Windows.
    if (m_prefs->screen_reader_external || m_prefs->screen_reader_sapi) Tolk_Unload();
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
void Core::init(bool dry_run)
{
    FileX::make_dir("userdata");
    FileX::make_dir("userdata/save");

    // Sets up the error-handling subsystem.
    m_guru_meditation = std::make_shared<Guru>("userdata/log.txt");

    // Sets up the random number generator.
    m_rng = std::make_shared<Random>();

    // Set up the user preferences.
    m_prefs = std::make_shared<Prefs>();

#ifdef GREAVE_TOLK
    // Set up Tolk if we're on Windows.
    if (m_prefs->screen_reader_sapi) Tolk_TrySAPI(true); // Enable SAPI.
    if (m_prefs->screen_reader_external || m_prefs->screen_reader_sapi) Tolk_Load();
    if (Tolk_DetectScreenReader())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Tolk_Silence();
    }
#endif

    std::string terminal_choice = StrX::str_tolower(m_prefs->terminal);
    if (!dry_run)
    {
#ifdef GREAVE_TARGET_WINDOWS
        if (terminal_choice != "curses") FreeConsole();
#endif

        // Set up our terminal emulator.
#ifdef GREAVE_INCLUDE_SDL
        if (terminal_choice == "sdl" || terminal_choice == "sdl2") m_terminal = std::make_shared<TerminalSDL2>();
        else
#endif
        if (terminal_choice == "curses") m_terminal = std::make_shared<TerminalCurses>();
        else m_guru_meditation->halt("Invalid terminal specified in prefs.yml");

        // Sets up the main message log window.
        m_message_log = std::make_shared<MessageLog>();

        // Tell the Guru system we're finished setting up the terminal and message window.
        guru()->console_ready();
    }

    // Sets up the text parser.
    m_parser = std::make_shared<Parser>();

    // Sets up the bones file.
    Bones::init_bones();

    // Load the help files.
    ActionHelp::load_pages();
}

// Loads a specified slot's saved game.
void Core::load(int save_slot)
{
    m_save_slot = save_slot;
    std::shared_ptr<SQLite::Database> save_db = std::make_shared<SQLite::Database>(save_filename(save_slot), SQLite::OPEN_READONLY);
    m_world->load(save_db);
}

// The main game loop.
void Core::main_loop()
{
    const auto player = m_world->player();
    // bröther may I have some lööps
    do
    {
        // For checking if the light level has changed due to something that happened this turn.
        const uint32_t location = player->location();
        const auto room = m_world->get_room(location);
        int old_light = room->light();

        const std::string input = m_message_log->render_message_log();
        m_parser->parse(input);

        // Check to see if the light level has changed.
        if (player->location() == location)
        {
            int new_light = room->light();
            if (old_light >= Room::LIGHT_VISIBLE && new_light < Room::LIGHT_VISIBLE) message("{u}You are plunged into {B}darkness{u}!");
            else if (old_light < Room::LIGHT_VISIBLE && new_light >= Room::LIGHT_VISIBLE)
            {
                message("{U}You can now see {W}clearly{U}!");
                ActionLook::look();
            }
        }
    } while (!player->is_dead());

    Bones::record_death();
    while (true)
    {
        message("{R}You are dead! Type {M}quit {R}when you are ready to end the game.");
        const std::string input = StrX::str_tolower(m_message_log->render_message_log());
        if (input == "quit") return;
    }
}

// Prints a message in the message log.
#ifdef GREAVE_TOLK
void Core::message(std::string msg, bool interrupt)
#else
void Core::message(std::string msg, bool)
#endif
{
    m_message_log->msg(msg);

#ifdef GREAVE_TOLK
    if (m_prefs->screen_reader_external || m_prefs->screen_reader_sapi)
    {
        if (m_prefs->screen_reader_process_square_brackets)
        {
            StrX::find_and_replace(msg, "[", "(");
            StrX::find_and_replace(msg, "]", ").");
        }
        std::regex filter("\\{[a-zA-Z0-9].?\\}");
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

// Returns a pointer to the Parser object.
const std::shared_ptr<Parser> Core::parser() const { return m_parser; }

// Returns a pointer to the Prefs object.
const std::shared_ptr<Prefs> Core::prefs() const { return m_prefs; }

// Returns a pointer to the Random object.
const std::shared_ptr<Random> Core::rng() const { return m_rng; }

// Saves the game to disk.
void Core::save()
{
    const std::string save_fn = save_filename(m_save_slot);
    const std::string save_fn_old = save_filename(m_save_slot, true);
    if (FileX::is_read_only(save_fn) || (FileX::file_exists(save_fn_old) && FileX::is_read_only(save_fn_old)))
    {
        core()->guru()->nonfatal("Saved game file is read-only!", Guru::GURU_ERROR);
        return;
    }
    if (FileX::file_exists(save_fn_old)) FileX::delete_file(save_fn_old);
    if (FileX::file_exists(save_fn))
    {
        FileX::rename_file(save_fn, save_fn_old);
        if (FileX::file_exists(save_fn))
        {
            m_guru_meditation->nonfatal("Could not rename saved game file. Is it read-only?", Guru::GURU_ERROR);
            return;
        }
    }

    try
    {
        std::shared_ptr<SQLite::Database> save_db = std::make_shared<SQLite::Database>(save_fn, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        save_db->exec("PRAGMA user_version = " + std::to_string(SAVE_VERSION));
        m_sql_unique_id = 0;    // We're making a new save file each time, so we can reset the unique ID counter.

        SQLite::Transaction transaction(*save_db);
        m_world->save(save_db);
        transaction.commit();

        message("{M}Game saved in slot {Y}" + std::to_string(m_save_slot) + "{M}.");
    } catch (std::exception &e)
    {
        m_guru_meditation->nonfatal("SQL error while attempting to save the game: " + std::string(e.what()), Guru::CRITICAL);
        if (FileX::file_exists(save_fn_old))
        {
            m_guru_meditation->nonfatal("Attempting to restore backup saved game file.", Guru::WARN);
            FileX::delete_file(save_fn);
            if (FileX::file_exists(save_fn)) m_guru_meditation->nonfatal("Could not delete current saved game file! Is it read-only?", Guru::GURU_ERROR);
            else FileX::rename_file(save_fn_old, save_fn);
        }
    }
}

// Returns a filename for a saved game file.
const std::string Core::save_filename(int slot, bool old_save) const { return "userdata/save/save-" + std::to_string(slot) + (old_save ? ".old" : ".sqlite"); }

// Checks the saved game version of a save file.
uint32_t Core::save_version(int slot)
{
    uint32_t version = 0;
    std::shared_ptr<SQLite::Database> save_db = std::make_shared<SQLite::Database>(save_filename(slot), SQLite::OPEN_READONLY);
    SQLite::Statement version_query(*save_db, "PRAGMA user_version");
    if (version_query.executeStep()) version = version_query.getColumn(0).getUInt();
    return version;
}

// Retrieves a new unique SQL ID.
uint32_t Core::sql_unique_id() { return ++m_sql_unique_id; }

// Returns a pointer  to the terminal emulator object.
const std::shared_ptr<Terminal> Core::terminal() const { return m_terminal; }

// The 'title screen' and saved game selection.
void Core::title()
{
    message("{U}Welcome to {G}Greave {U}" + GAME_VERSION +
        ", copyright (c) 2021 Raine \"Gravecat\" Simmons and the Greave contributors. This game is free and open-source, released under the Gnu AGPL 3.0 license.");
#ifdef GREAVE_TOLK
    if (Tolk_DetectScreenReader()) message("{U}If you are using a screen reader, pressing the {C}tab key {U}will repeat the text after your last input.");
#endif

    std::vector<bool> save_exists;
    save_exists.resize(m_prefs->save_file_slots);
    bool deleting_file = false;
    while (!m_save_slot)
    {
        if (deleting_file)
        {
            message("{R}Please select which saved game to delete:");
            message("{U}[{C}C{U}] {R}Cancel, do not delete");
        }
        else
        {
            message("{U}Please select a saved game slot to begin the game:");
            message("{U}[{C}D{U}] {R}Delete a saved game");
            message("{0}{U}[{C}Q{U}] {R}Quit game");
            message("{0}{U}[{C}L{U}] {W}Hall of Legends");
        }
        for (int i = 1; i <= m_prefs->save_file_slots; i++)
        {
            if (FileX::file_exists(save_filename(i)))
            {
                std::string save_str = "Saved game #" + std::to_string(i);
                const uint32_t save_ver = save_version(i);
                if (save_ver == SAVE_VERSION) save_str = "{W}" + save_str;
                else save_str = "{R}" + save_str + " {M}<incompatible>";
                message("{0}{U}[{C}" + std::to_string(i) + "{U}] " + save_str);
                save_exists.at(i - 1) = true;
            }
            else
            {
                message("{0}{U}[{C}" + std::to_string(i) + "{U}] {B}Empty slot #" + std::to_string(i));
                save_exists.at(i - 1) = false;
            }
        }

        bool inner_loop = true;
        int patience_counter = 0;
        while (inner_loop)
        {
            std::string input = messagelog()->render_message_log();
            if (!input.size()) continue;
            if (input.size() >= 3 && (input[0] == '[' || input[0] == '(')) input = input.substr(1);

            if ((input[0] == 'q' || input[0] == 'Q') && !deleting_file)
            {
                core()->cleanup();
                exit(EXIT_SUCCESS);
            }
            else if ((input[0] == 'd' || input[0] == 'D') && !deleting_file)
            {
                patience_counter = 0;
                deleting_file = true;
                inner_loop = false;
            }
            else if ((input[0] == 'c' || input[0] == 'C') && deleting_file)
            {
                patience_counter = 0;
                deleting_file = false;
                inner_loop = false;
                message("{U}Okay, no save file will be deleted.");
            }
            else if ((input[0] == 'l' || input[0] == 'L') && !deleting_file)
            {
                inner_loop = false;
                Bones::hall_of_legends();
            }
            else
            {
                int input_num = input[0] - '0';
                if (input_num < 1 || input_num > static_cast<int>(m_prefs->save_file_slots))
                {
                    if (++patience_counter > 5)
                    {
                        message("{y}That is not a valid option.");
                        inner_loop = false;
                    }
                    else
                    {
                        if (deleting_file) message("{y}That is not a valid option. Please choose {Y}a save slot number{y} or {Y}C{y}.");
                        else message("{y}That is not a valid option. Please choose {Y}a save slot number{y}, {Y}D{y}, {Y}Q{y} or {Y}L{y}.");
                    }
                }
                else
                {
                    if (deleting_file)
                    {
                        if (!FileX::file_exists(save_filename(input_num)))
                        {
                            message("{y}There isn't a saved game in that slot.");
                            inner_loop = false;
                        }
                        else
                        {
                            message("{R}Are you sure you want to delete saved game {W}#" + std::to_string(input_num) + "{R}? This decision cannot be undone! {M}[{R}Y{r}/{R}N{M}]");
                            bool yes_no_loop = true;
                            while (yes_no_loop)
                            {
                                std::string yes_no = messagelog()->render_message_log();
                                if (yes_no.size())
                                {
                                    if (yes_no[0] == 'y' || yes_no[0] == 'Y')
                                    {
                                        inner_loop = yes_no_loop = deleting_file = false;
                                        FileX::delete_file(save_filename(input_num));
                                        if (FileX::file_exists(save_filename(input_num, true))) FileX::delete_file(save_filename(input_num, true));
                                        message("{M}Save file {W}#" + std::to_string(input_num) + " {M}has been deleted!");
                                    }
                                    else if (yes_no[0] == 'n' || yes_no[0] == 'N')
                                    {
                                        message("{U}Okay, this save file will not be deleted.");
                                        inner_loop = yes_no_loop = false;
                                    }
                                    else message("{R}Please choose either {M}YES {R}or {M}NO{R}.");
                                }
                            }
                        }
                    }
                    else
                    {
                        uint32_t save_file_ver = 0;
                        const bool file_exists = FileX::file_exists(save_filename(input_num));
                        if (file_exists) save_file_ver = save_version(input_num);
                        if (!file_exists || save_file_ver == SAVE_VERSION)
                        {
                            m_save_slot = input_num;
                            inner_loop = false;
                        }
                        else message("{R}This saved game is {M}incompatible {R}with this version of the game. Greave " + GAME_VERSION + " uses save file {M}version " +
                                std::to_string(SAVE_VERSION) + "{R}, this save file is using {M}" + (save_file_ver ? "version " + std::to_string(save_file_ver) :
                                "an unknown version") + "{R}.");
                    }
                }
            }
        }
    }

    if (save_exists.at(m_save_slot - 1))
    {
        m_guru_meditation->cache_nonfatal();
        m_world = std::make_shared<World>();
        load(m_save_slot);
        m_guru_meditation->dump_nonfatal();
    }
    else
    {
        m_world = std::make_shared<World>();
        m_world->new_game();
    }
}

// Returns a pointer to the World object.
const std::shared_ptr<World> Core::world() const { return m_world; }

// Allows external access to the Core object.
const std::shared_ptr<Core> core()
{
    if (!greave) exit(EXIT_FAILURE);
    else return greave;
}
