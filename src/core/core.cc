// core/core.cc -- Main program entry, initialization and cleanup routines, and the core game loop.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#ifdef GREAVE_TOLK
#include "3rdparty/Tolk/Tolk.h"
#endif
#include "actions/help.h"
#include "core/core.h"
#include "core/core-constants.h"
#include "core/bones.h"
#include "core/filex.h"
#include "core/strx.h"
#include "core/terminal-curses.h"
#include "core/terminal-sdl2.h"

#ifdef GREAVE_TOLK
#include <regex>
#endif
#include <thread>
#ifdef GREAVE_TARGET_WINDOWS
#include <windows.h>
#endif


std::shared_ptr<Core> greave = nullptr;   // The main Core object.


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
Core::Core() : message_log_(nullptr), parser_(nullptr), rng_(nullptr), save_slot_(0), sql_unique_id_(0), terminal_(nullptr), prefs_(nullptr), world_(nullptr) { }

// Cleans up after we're d one.
void Core::cleanup()
{
    // Tell Guru to revert to exit() if an error happens at this point.
    guru()->console_ready(false);

#ifdef GREAVE_TOLK
    // Clean up Tolk, if we're on Windows.
    if (prefs_->screen_reader_external || prefs_->screen_reader_sapi) Tolk_Unload();
#endif

    terminal_ = nullptr;   // It's a smart pointer, so this'll run the destructor code.
}

// Returns a pointer to the Guru Meditation object.
const std::shared_ptr<Guru> Core::guru() const
{
    if (!guru_meditation_) exit(EXIT_FAILURE);
    return guru_meditation_;
}

// Sets up the core game classes and data.
void Core::init(bool dry_run)
{
    FileX::make_dir("userdata");
    FileX::make_dir("userdata/save");

    // Sets up the error-handling subsystem.
    guru_meditation_ = std::make_shared<Guru>("userdata/log.txt");

    // Sets up the random number generator.
    rng_ = std::make_shared<Random>();

    // Set up the user preferences.
    prefs_ = std::make_shared<Prefs>();

#ifdef GREAVE_TOLK
    // Set up Tolk if we're on Windows.
    if (prefs_->screen_reader_sapi) Tolk_TrySAPI(true); // Enable SAPI.
    if (prefs_->screen_reader_external || prefs_->screen_reader_sapi) Tolk_Load();
    if (Tolk_DetectScreenReader())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Tolk_Silence();
    }
#endif

    std::string terminal_choice = StrX::str_tolower(prefs_->terminal);
    if (!dry_run)
    {

#if !defined(GREAVE_INCLUDE_SDL) && !defined(GREAVE_INCLUDE_CURSES)
#error No support for either SDL2 or Curses! Please choose at least one!
#endif

        // If SDL or Curses is disabled, just default to the other option.
#ifndef GREAVE_INCLUDE_SDL
        terminal_choice = "curses";
#elif !defined(GREAVE_INCLUDE_CURSES)
        terminal_choice = "sdl2";
#endif

        // Set up our terminal emulator.
#ifdef GREAVE_INCLUDE_SDL
        if (terminal_choice == "sdl" || terminal_choice == "sdl2")
        {
            try
            {
                terminal_ = std::make_shared<TerminalSDL2>();
            }
            catch (std::exception &e)
            {
#ifdef GREAVE_INCLUDE_CURSES
                core()->guru()->log("Could not initialize SDL terminal! Falling back to Curses...", Guru::GURU_WARN);
                terminal_choice = "curses";
#else
                throw e;
#endif
            }
        }
#endif
#ifdef GREAVE_INCLUDE_CURSES
        if (terminal_choice == "curses") terminal_ = std::make_shared<TerminalCurses>();
#endif
        if (!terminal_) guru_meditation_->halt("Invalid terminal specified in prefs.yml");

#ifdef GREAVE_TARGET_WINDOWS
#ifdef GREAVE_INCLUDE_CURSES
        if (terminal_choice != "curses") FreeConsole();
#endif
#endif

        // Sets up the main message log window.
        message_log_ = std::make_shared<MessageLog>();

        // Tell the Guru system we're finished setting up the terminal and message window.
        guru()->console_ready();
    }

    // Sets up the text parser.
    parser_ = std::make_shared<Parser>();

    // Sets up the bones file.
    Bones::init_bones();

    // Load the help files.
    ActionHelp::load_pages();
}

// Loads a specified slot's saved game.
void Core::load(int save_slot)
{
    save_slot_ = save_slot;
    std::shared_ptr<SQLite::Database> save_db = std::make_shared<SQLite::Database>(save_filename(save_slot), SQLite::OPEN_READONLY);
    world_->load(save_db);
}

// The main game loop.
void Core::main_loop()
{
    const auto player = world_->player();
    // bröther may I have some lööps
    do
    {
        world_->main_loop_events_pre_input();
        const std::string input = message_log_->render_message_log();
        parser_->parse(input);
        world_->main_loop_events_post_input();
    } while (!player->is_dead());

    Bones::record_death();
    while (true)
    {
        message("{R}You are dead! Type {M}quit {R}when you are ready to end the game.");
        const std::string input = StrX::str_tolower(message_log_->render_message_log());
        if (input == "quit") return;
    }
}

// Prints a message in the message log.
void Core::message(std::string msg, bool interrupt)
{
    message_log_->msg(msg);
    screen_read(msg, interrupt);
}

// Reads a string in a screen reader, if any are active.
#ifdef GREAVE_TOLK
void Core::screen_read(std::string msg, bool interrupt)
{
    if (prefs_->screen_reader_external || prefs_->screen_reader_sapi)
    {
        if (prefs_->screen_reader_process_square_brackets)
        {
            StrX::find_and_replace(msg, "[", "(");
            StrX::find_and_replace(msg, "]", ").");
        }
        std::regex filter("\\{[a-zA-Z0-9].?\\}");
        std::string msg_voice = std::regex_replace(msg, filter, "");
        if (msg_voice.size() >= 2 && msg_voice[0] == '>') msg_voice = msg_voice.substr(2);
        const std::wstring msg_wide(msg_voice.begin(), msg_voice.end());
        Tolk_Output(msg_wide.c_str(), interrupt);
        if (interrupt) message_log_->clear_latest_messages();
        message_log_->add_latest_message(msg_voice);
    }
}
#else
void Core::screen_read(std::string, bool) { }
#endif

// Returns a pointer to the MessageLog object.
const std::shared_ptr<MessageLog> Core::messagelog() const { return message_log_; }

// Returns a pointer to the Parser object.
const std::shared_ptr<Parser> Core::parser() const { return parser_; }

// Returns a pointer to the Prefs object.
const std::shared_ptr<Prefs> Core::prefs() const { return prefs_; }

// Returns a pointer to the Random object.
const std::shared_ptr<Random> Core::rng() const { return rng_; }

// Saves the game to disk.
void Core::save()
{
    const std::string save_fn = save_filename(save_slot_);
    const std::string save_fn_old = save_filename(save_slot_, true);
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
            guru_meditation_->nonfatal("Could not rename saved game file. Is it read-only?", Guru::GURU_ERROR);
            return;
        }
    }

    try
    {
        std::shared_ptr<SQLite::Database> save_db = std::make_shared<SQLite::Database>(save_fn, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        save_db->exec("PRAGMA user_version = " + std::to_string(CoreConstants::SAVE_VERSION));
        sql_unique_id_ = 0; // We're making a new save file each time, so we can reset the unique ID counter.

        SQLite::Transaction transaction(*save_db);
        world_->save(save_db);
        transaction.commit();

        message("{M}Game saved in slot {Y}" + std::to_string(save_slot_) + "{M}.");
    } catch (std::exception &e)
    {
        guru_meditation_->nonfatal("SQL error while attempting to save the game: " + std::string(e.what()), Guru::GURU_CRITICAL);
        if (FileX::file_exists(save_fn_old))
        {
            guru_meditation_->nonfatal("Attempting to restore backup saved game file.", Guru::GURU_WARN);
            FileX::delete_file(save_fn);
            if (FileX::file_exists(save_fn)) guru_meditation_->nonfatal("Could not delete current saved game file! Is it read-only?", Guru::GURU_ERROR);
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
uint32_t Core::sql_unique_id() { return ++sql_unique_id_; }

// Returns a pointer  to the terminal emulator object.
const std::shared_ptr<Terminal> Core::terminal() const { return terminal_; }

// The 'title screen' and saved game selection.
void Core::title()
{
    message("{U}Welcome to {G}Greave {U}" + std::string(CoreConstants::GAME_VERSION) + ", copyright (c) 2021 Raine \"Gravecat\" Simmons and the Greave contributors. This game is free and open-source, released under the Gnu AGPL 3.0 license.");
#ifdef GREAVE_TOLK
    if (Tolk_DetectScreenReader()) message("{U}If you are using a screen reader, pressing the {C}tab key {U}will repeat the text after your last input, and pressing {C}escape {U}at any time will stop reading.");
#endif

    std::vector<bool> save_exists;
    save_exists.resize(prefs_->save_file_slots);
    bool deleting_file = false;
    while (!save_slot_)
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
        for (int i = 1; i <= prefs_->save_file_slots; i++)
        {
            if (FileX::file_exists(save_filename(i)))
            {
                std::string save_str = "Saved game #" + std::to_string(i);
                const uint32_t save_ver = save_version(i);
                if (save_ver == CoreConstants::SAVE_VERSION) save_str = "{W}" + save_str;
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
                if (input_num < 1 || input_num > static_cast<int>(prefs_->save_file_slots))
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
                        if (!file_exists || save_file_ver == CoreConstants::SAVE_VERSION)
                        {
                            save_slot_ = input_num;
                            inner_loop = false;
                        }
                        else message("{R}This saved game is {M}incompatible {R}with this version of the game. Greave " + std::string(CoreConstants::GAME_VERSION) + " uses save file {M}version " + std::to_string(CoreConstants::SAVE_VERSION) + "{R}, this save file is using {M}" + (save_file_ver ? "version " + std::to_string(save_file_ver) : "an unknown version") + "{R}.");
                    }
                }
            }
        }
    }

    if (save_exists.at(save_slot_ - 1))
    {
        guru_meditation_->cache_nonfatal();
        world_ = std::make_shared<World>();
        load(save_slot_);
        guru_meditation_->dump_nonfatal();
    }
    else
    {
        world_ = std::make_shared<World>();
        world_->new_game();
    }
}

// Returns a pointer to the World object.
const std::shared_ptr<World> Core::world() const { return world_; }

// Allows external access to the Core object.
const std::shared_ptr<Core> core()
{
    if (!greave) exit(EXIT_FAILURE);
    else return greave;
}
