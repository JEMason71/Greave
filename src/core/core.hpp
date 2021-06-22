// core/core.hpp -- Main program entry, initialization and cleanup routines, and the core game loop.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "uni/uni-system.hpp"

class Guru;     // defined in core/guru.hpp
class Terminal; // defined in terminal/terminal.hpp
class Tune;     // defined in core/tune.hpp


class GreaveCore
{
public:
                                    GreaveCore();       // Constructor, doesn't do too much aside from setting default values for member variables. Use init() to set things up.
    void                            cleanup();          // Cleans up after we're done.
    const std::shared_ptr<Guru>     guru() const;       // Returns a pointer to the Guru Meditation object.
    void                            init();             // Sets up the core game classes and data.
    const std::shared_ptr<Terminal> terminal() const;   // Returns a pointer  to the terminal emulator object.
    const std::shared_ptr<Tune>     tune() const;       // Returns a pointer to the Tune object.

    static const std::string    GAME_VERSION;   // The game's version number.

private:
    std::shared_ptr<Guru>       m_guru_meditation;  // The Guru Meditation error-handling system.
    std::shared_ptr<Terminal>   m_terminal;         // The Terminal class, which handles low-level interaction with terminal emulation libraries.
    std::shared_ptr<Tune>       m_tune;             // The Tune object, containing various tweakable numbers in tune.yml
};


const std::shared_ptr<GreaveCore>   core(); // Allows external access to the main GreaveCore object.
