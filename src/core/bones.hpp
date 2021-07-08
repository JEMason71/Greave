// core/bones.hpp -- Systems related to the player character's death, the highscore table, and recording data about the dead character that may be used in future games.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"


class Bones
{
public:
    static void     hall_of_legends();  // Displays the Hall of Legends highscore table.
    static void     init_bones();       // Initializes the bones file, creating a new file if needed.
    static bool     record_death();     // Record the player's death in the bones file.
    static uint32_t unique_id();        // Returns a random player ID which isn't already present in the bones file.

private:
    static const std::string    BONES_FILENAME; // The filename for the bones file.
    static const uint32_t       BONES_VERSION;  // The expected version format for the bones file.
    static const int            MAX_HIGHSCORES; // The maximum amount of highscores to store.
    static const std::string    SQL_BONES;      // SQL table construction string.

    static uint32_t bones_version();    // Checks the version of the bones file, 0 if the file doesn't exist or version cannot be determined.
};
