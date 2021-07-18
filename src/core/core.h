// core/core.h -- Main program entry, initialization and cleanup routines, and the core game loop.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_CORE_CORE_H_
#define GREAVE_CORE_CORE_H_

#include "core/guru.h"
#include "core/message.h"
#include "core/parser.h"
#include "core/prefs.h"
#include "core/random.h"
#include "core/terminal.h"
#include "world/world.h"

#include <cstdint>
#include <memory>
#include <string>


class Core
{
public:
                                        Core();                 // Constructor, doesn't do too much aside from setting default values for member variables. Use init() to set things up.
    void                                cleanup();              // Cleans up after we're done.
    const std::shared_ptr<Guru>         guru() const;           // Returns a pointer to the Guru Meditation object.
    void                                init(bool dry_run);     // Sets up the core game classes and data.
    void                                load(int save_slot);    // Loads a specified slot's saved game.
    void                                main_loop();            // The main game loop.
    void                                message(std::string msg, bool interrupt = false);   // Prints a message.
    const std::shared_ptr<MessageLog>   messagelog() const;     // Returns a pointer to the MessageLog object.
    const std::shared_ptr<Parser>       parser() const;         // Returns a pointer to the Parser object.
    const std::shared_ptr<Random>       rng() const;            // Returns a pointer to the Random object.
    void                                save();                 // Saves the game to disk.
    void                                screen_read(std::string msg, bool interrupt);   // Reads a string in a screen reader, if any are active.
    uint32_t                            sql_unique_id();        // Retrieves a new unique SQL ID.
    const std::shared_ptr<Terminal>     terminal() const;       // Returns a pointer  to the terminal emulator object.
    void                                title();                // The 'title screen' and saved game selection.
    const std::shared_ptr<Prefs>        prefs() const;          // Returns a pointer to the Prefs object.
    const std::shared_ptr<World>        world() const;          // Returns a pointer to the World object.

private:
    const std::string           save_filename(int slot, bool old_save = false) const;   // Returns a filename for a saved game file.
    uint32_t                    save_version(int slot); // Checks the saved game version of a save file.

    std::shared_ptr<Guru>       m_guru_meditation;  // The Guru Meditation error-handling system.
    std::shared_ptr<MessageLog> m_message_log;      // The MessageLog object, which handles the scrolling message-log input/output window.
    std::shared_ptr<Parser>     m_parser;           // The Parser object, which processes the player's input.
    std::shared_ptr<Random>     m_rng;              // The random number generator.
    int                         m_save_slot;        // The currently-active saved game slot, or 0 if no game is in progress.
    uint32_t                    m_sql_unique_id;    // The last unique SQL ID to have been used.
    std::shared_ptr<Terminal>   m_terminal;         // The Terminal class, which handles low-level interaction with terminal emulation libraries.
    std::shared_ptr<Prefs>      m_prefs;            // The Prefs object, containing various user settings in prefs.yml
    std::shared_ptr<World>      m_world;            // The World object, which manages the current overall state of the game.
};


const std::shared_ptr<Core> core(); // Allows external access to the main Core object.

#endif  // GREAVE_CORE_CORE_H_
