// core/core.cpp -- Main program entry, initialization and cleanup routines, and the core game loop.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "core/core.hpp"
#include "uni/uni-core.hpp"
#include "uni/uni-tolk.hpp"

#include <thread>
#ifdef GREAVE_TARGET_WINDOWS
#include <windows.h>
#endif


std::shared_ptr<GreaveCore> greave = nullptr;	// The main GreaveCore object.


// Main program entry point.
int main(int argc, char* argv[])
{
    // Check command-line parameters.
    std::vector<std::string> parameters(argv, argv + argc);

    greave = std::make_shared<GreaveCore>();
    greave->init();
    greave->cleanup();
    return EXIT_SUCCESS;
}

// Constructor, doesn't do too much aside from setting default values for member variables. Use init() to set things up.
GreaveCore::GreaveCore() : m_tune(nullptr) { }

// Cleans up after we're d one.
void GreaveCore::cleanup()
{
    // Tell Guru to revert to exit() if an error happens at this point.
    guru()->console_ready(false);

#ifdef GREAVE_TOLK
    // Clean up Tolk, if we're on Windows.
    if (m_tune->screen_reader_external || m_tune->screen_reader_sapi) Tolk_Unload();
#endif
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
#endif

    // Tell the Guru system we're finished setting up.
    guru()->console_ready();
}

// Returns a pointer to the Tune object.
const std::shared_ptr<Tune> GreaveCore::tune() const { return m_tune; }

// Allows external access to the GreaveCore object.
const std::shared_ptr<GreaveCore> core()
{
    if (!greave) exit(EXIT_FAILURE);
    else return greave;
}
