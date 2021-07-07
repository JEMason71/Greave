// core/bones.cpp -- Systems related to the player character's death, the highscore table, and recording data about the dead character that may be used in future games.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/bones.hpp"
#include "core/core.hpp"
#include "core/filex.hpp"
#include "core/guru.hpp"
#include "core/message.hpp"
#include "core/strx.hpp"
#include "world/player.hpp"
#include "world/world.hpp"


const std::string   Bones::BONES_FILENAME = "userdata/bones.sqlite";    // The filename for the bones file.
const uint32_t      Bones::BONES_VERSION =  1;  // The expected version format for the bones file.
const int           Bones::MAX_HIGHSCORES = 10; // The maximum amount of highscores to store.

// SQL table construction string.
const std::string   Bones::SQL_BONES =  "CREATE TABLE highscores ( death_reason TEXT NOT NULL, id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, name TEXT NOT NULL, "
    "score INTEGER NOT NULL )";


// Displays the Hall of Legends highscore table.
void Bones::hall_of_legends()
{
    SQLite::Database bones_db(BONES_FILENAME, SQLite::OPEN_READONLY);
    int scores = bones_db.execAndGet("SELECT COUNT(*) FROM highscores LIMIT " + std::to_string(Bones::MAX_HIGHSCORES)).getInt();
    if (scores > MAX_HIGHSCORES) scores = MAX_HIGHSCORES;
    if (!scores)
    {
        core()->message("{Y}The Hall of Legends is barren and empty, awaiting fallen heroes of note.");
        return;
    }
    std::string heroes_str = "These heroes are";
    if (scores == 1) heroes_str = "This hero is";
    core()->message("{G}" + heroes_str + " recorded in the hallowed tomes of the {R}H{Y}a{G}l{C}l {U}o{M}f {R}L{Y}e{G}g{C}e{U}n{M}d{R}s{G}:");

    SQLite::Statement query(bones_db, "SELECT death_reason, name, score FROM highscores ORDER BY score DESC LIMIT " + std::to_string(Bones::MAX_HIGHSCORES));
    int position = 0;
    while (query.executeStep())
    {
        position++;
        const std::string death_reason = query.getColumn("death_reason").getString();
        const std::string name = query.getColumn("name").getString();
        const uint32_t score = query.getColumn("score").getUInt();
        std::string colour = "{r}";
        if (position == 1) colour = "{G}";
        else if (position == 2) colour = "{C}";
        else if (position <= 4) colour = "{U}";
        else if (position <= 6) colour = "{M}";
        else if (position <= 8) colour = "{R}";
        std::string pos_str = std::to_string(position);
        if (pos_str.size() == 1 && scores >= 10) pos_str = "`" + pos_str;
        core()->message((position == 1 ? "" : "{0}") + colour + " " + pos_str + ". " + name + colour + " - " + StrX::intostr_pretty(score) + " - " + death_reason);
    }

    core()->message("{U}Type anything then hit enter to return to the main menu.");
    core()->messagelog()->render_message_log();
}

// Initializes the bones file, creating a new file if needed.
void Bones::init_bones()
{
    bool recreate_bones_file = false;
    if (!FileX::file_exists(BONES_FILENAME)) recreate_bones_file = true;
    else
    {
        // Check if an existing bones file is valid and the correct version.
        core()->guru()->log("Validating bones file.");
        try
        {
            uint32_t version = 0;
            SQLite::Database bones_db(BONES_FILENAME, SQLite::OPEN_READONLY);
            SQLite::Statement version_query(bones_db, "PRAGMA user_version");
            if (version_query.executeStep()) version = version_query.getColumn(0).getUInt();
            if (version != BONES_VERSION) recreate_bones_file = true;
        }
        catch(const std::exception &e)
        {
            recreate_bones_file = true;
        }
    }
    if (!recreate_bones_file) return;

    // If the old file exists, delete it. If it doesn't want to be deleted, we'll have to just error out.
    if (FileX::file_exists(BONES_FILENAME))
    {
        core()->guru()->log("Removing invalid or incorrect version bones file.");
        FileX::delete_file(BONES_FILENAME);
    }
    if (FileX::file_exists(BONES_FILENAME)) throw std::runtime_error("Could not delete invalid bones.sqlite file!");

    // Create a new, clean bones file.
    core()->guru()->log("Creating fresh bones file.");
    SQLite::Database bones_db(BONES_FILENAME, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    bones_db.exec("PRAGMA user_version = " + std::to_string(BONES_VERSION));
    bones_db.exec(SQL_BONES);
}

// Record the player's death in the bones file.
bool Bones::record_death()
{
    int belongs_in_hall_of_legends = 0;
    const auto player = core()->world()->player();
    if (player->score())
    {
        try
        {
            SQLite::Database bones_db(BONES_FILENAME, SQLite::OPEN_READWRITE);
            int scores_checked = 0;
            SQLite::Statement query(bones_db, "SELECT score FROM highscores ORDER BY score DESC LIMIT " + std::to_string(MAX_HIGHSCORES));
            while (query.executeStep())
            {
                scores_checked++;
                if (!belongs_in_hall_of_legends && query.getColumn("score").getUInt() < player->score()) belongs_in_hall_of_legends = scores_checked;
            }
            if (!scores_checked) belongs_in_hall_of_legends = 1;
            else if (scores_checked < MAX_HIGHSCORES && !belongs_in_hall_of_legends) belongs_in_hall_of_legends = scores_checked + 1;
            if (belongs_in_hall_of_legends)
            {
                SQLite::Statement insert(bones_db, "INSERT INTO highscores ( death_reason, name, score ) VALUES ( ?, ?, ? )");
                insert.bind(1, player->death_reason());
                insert.bind(2, player->name());
                insert.bind(3, player->score());
                insert.exec();
                if (scores_checked >= MAX_HIGHSCORES)
                    bones_db.exec("DELETE FROM highscores WHERE ID NOT IN (SELECT id FROM highscores ORDER BY score DESC LIMIT " + std::to_string(MAX_HIGHSCORES) + ")");
            }
        }
        catch (std::exception &e)
        {
            core()->guru()->nonfatal("Could not record player death in bones file!", Guru::ERROR);
            return false;
        }
    }
    if (belongs_in_hall_of_legends) core()->message("{G}Your name was recorded in the {R}H{Y}a{G}l{C}l {U}o{M}f {R}L{Y}e{G}g{C}e{U}n{M}d{R}s{G}!");
    return belongs_in_hall_of_legends;
}
