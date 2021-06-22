// core/core.cpp -- Main program entry, initialization and cleanup routines, and the core game loop.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "core/core.hpp"
#include "uni/uni-core.hpp"

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
GreaveCore::GreaveCore() { }

// Cleans up after we're d one.
void GreaveCore::cleanup()
{
    // nothing here right now
}

// Sets up the core game classes and data.
void GreaveCore::init()
{
    // nothing here right now
}
