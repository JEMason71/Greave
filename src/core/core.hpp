// core/core.hpp -- Main program entry, initialization and cleanup routines, and the core game loop.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "uni/uni-system.hpp"


class GreaveCore
{
public:
            GreaveCore();   // Constructor, doesn't do too much aside from setting default values for member variables. Use init() to set things up.
    void    cleanup();      // Cleans up after we're done.
    void    init();         // Sets up the core game classes and data.
};


const std::shared_ptr<GreaveCore>   core(); // Allows external access to the main GreaveCore object.
