// core/core.hpp -- Main program entry, initialization and cleanup routines, and the core game loop.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Guru;         // defined in core/guru.hpp
class MessageLog;   // defined in core/message.hpp
class Terminal;     // defined in terminal/terminal.hpp
class Tune;         // defined in core/tune.hpp
class World;        // defined in core/world.hpp


class Core
{
public:
                                        Core();             // Constructor, doesn't do too much aside from setting default values for member variables. Use init() to set things up.
    void                                cleanup();          // Cleans up after we're done.
    const std::shared_ptr<Guru>         guru() const;       // Returns a pointer to the Guru Meditation object.
    void                                init();             // Sets up the core game classes and data.
    void                                message(std::string msg, uint32_t flags = 0);   // Prints a message in the message log.
    const std::shared_ptr<MessageLog>   messagelog() const; // Returns a pointer to the MessageLog object.
    void                                play();             // Starts the game.
    const std::shared_ptr<Terminal>     terminal() const;   // Returns a pointer  to the terminal emulator object.
    const std::shared_ptr<Tune>         tune() const;       // Returns a pointer to the Tune object.
    const std::shared_ptr<World>        world() const;      // Returns a pointer to the World object.

    static const std::string    GAME_VERSION;       // The game's version number.
    static const uint32_t       MSG_FLAG_INTERRUPT; // Flags for the message() function.

private:
    void    main_loop();    // The main game loop.

    std::shared_ptr<Guru>       m_guru_meditation;  // The Guru Meditation error-handling system.
    std::shared_ptr<MessageLog> m_message_log;      // The MessageLog object, which handles the scrolling message-log input/output window.
    std::shared_ptr<Terminal>   m_terminal;         // The Terminal class, which handles low-level interaction with terminal emulation libraries.
    std::shared_ptr<Tune>       m_tune;             // The Tune object, containing various tweakable numbers in tune.yml
    std::shared_ptr<World>      m_world;            // The World object, which manages the current overall state of the game.
};


const std::shared_ptr<Core> core(); // Allows external access to the main Core object.
