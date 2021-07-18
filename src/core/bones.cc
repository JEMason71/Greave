// core/bones.cc -- Systems related to the player character's death, the highscore table, and recording data about the dead character that may be used in future games.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/bones.h"
#include "core/core.h"
#include "core/filex.h"
#include "core/strx.h"

#include <string>


// The filename for the bones file.
constexpr char Bones::BONES_FILENAME[] = "userdata/bones.sqlite";

// SQL table construction string.
constexpr char Bones::SQL_BONES[] = "CREATE TABLE highscores ( death_reason TEXT NOT NULL, id INTEGER PRIMARY KEY UNIQUE NOT NULL, name TEXT NOT NULL, score INTEGER NOT NULL )";


// Checks the version of the bones file, 0 if the file doesn't exist or version cannot be determined.
uint32_t Bones::bones_version()
{
    try
    {
        if (!FileX::file_exists(BONES_FILENAME)) return 0;
        SQLite::Database bones_db(BONES_FILENAME, SQLite::OPEN_READONLY);
        SQLite::Statement version_query(bones_db, "PRAGMA user_version");
        if (version_query.executeStep()) return version_query.getColumn(0).getUInt();
        else return 0;
    }
    catch (std::exception &e)
    {
        return 0;
    }
}

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
    core()->guru()->log("Validating bones file.");
    if (bones_version() == BONES_VERSION) return;

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

            // First, check if this player ID is already present on the scoreboard.
            SQLite::Statement duplicate_query(bones_db, "SELECT id FROM highscores WHERE id = :id");
            duplicate_query.bind(":id", player->meta_uint("bones_id"));
            if (duplicate_query.executeStep())
            {
                SQLite::Statement update(bones_db, "UPDATE highscores SET death_reason = :death_reason, name = :name, score = :score WHERE id = :bones_id");
                update.bind(":death_reason", player->death_reason());
                update.bind(":name", player->name());
                update.bind(":score", player->score());
                update.bind(":bones_id", player->meta_uint("bones_id"));
                update.exec();
                belongs_in_hall_of_legends = 1;
            }
            else
            {
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
                    SQLite::Transaction transaction(bones_db);
                    SQLite::Statement insert(bones_db, "INSERT INTO highscores ( death_reason, id, name, score ) VALUES ( :death_reason, :id, :name, :score )");
                    insert.bind(":death_reason", player->death_reason());
                    insert.bind(":id", player->meta_uint("bones_id"));
                    insert.bind(":name", player->name());
                    insert.bind(":score", player->score());
                    insert.exec();
                    if (scores_checked >= MAX_HIGHSCORES)
                        bones_db.exec("DELETE FROM highscores WHERE ID NOT IN (SELECT id FROM highscores ORDER BY score DESC LIMIT " + std::to_string(MAX_HIGHSCORES) + ")");
                    transaction.commit();
                }
            }
        }
        catch (std::exception &e)
        {
            core()->guru()->nonfatal("Could not record player death in bones file! " + std::string(e.what()), Guru::GURU_ERROR);
            return false;
        }
    }
    if (belongs_in_hall_of_legends) core()->message("{G}Your name was recorded in the {R}H{Y}a{G}l{C}l {U}o{M}f {R}L{Y}e{G}g{C}e{U}n{M}d{R}s{G}!");
    return belongs_in_hall_of_legends;
}

// Returns a random player ID which isn't already present in the bones file.
uint32_t Bones::unique_id()
{
    try
    {
        if (bones_version() == BONES_VERSION)
        {
            bool valid = true;
            uint32_t choice;
            SQLite::Database bones_db(BONES_FILENAME, SQLite::OPEN_READONLY);
            do
            {
                choice = core()->rng()->rnd(UINT32_MAX);
                SQLite::Statement query(bones_db, "SELECT id FROM highscores WHERE id = :id");
                query.bind(":id", choice);
                if (query.executeStep()) valid = false;
                else valid = true;
            } while (!valid);
            return choice;
        }
    }
    catch (std::exception&) { }
    return core()->rng()->rnd(UINT32_MAX);
}
