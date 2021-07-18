// core/bones.h -- Systems related to the player character's death, the highscore table, and recording data about the dead character that may be used in future games.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_CORE_BONES_H_
#define GREAVE_CORE_BONES_H_

#include <cstddef>


class Bones
{
public:
    static void     hall_of_legends();  // Displays the Hall of Legends highscore table.
    static void     init_bones();       // Initializes the bones file, creating a new file if needed.
    static bool     record_death();     // Record the player's death in the bones file.
    static uint32_t unique_id();        // Returns a random player ID which isn't already present in the bones file.

private:
    static constexpr uint32_t   BONES_VERSION =     1;  // The expected version format for the bones file.
    static constexpr int        MAX_HIGHSCORES =    10; // The maximum amount of highscores to store.
    static const char           BONES_FILENAME[];       // The filename for the bones file.
    static const char           SQL_BONES[];            // SQL table construction string.

    static uint32_t bones_version();    // Checks the version of the bones file, 0 if the file doesn't exist or version cannot be determined.
};

#endif  // GREAVE_CORE_BONES_H_
