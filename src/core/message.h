// core/message.h -- The main interface to the game, a scrolling message log and an input window.
// Copyright (c) 2019-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.
// Originally based on BearLibTerminal sample code, (c) 2014 Cfyz.

#ifndef GREAVE_CORE_MESSAGE_H_
#define GREAVE_CORE_MESSAGE_H_

#include "3rdparty/SQLiteCpp/Database.h"

#include <string>
#include <vector>


class MessageLog
{
public:
    static const char   SQL_MSGLOG[];   // SQL string to construct database table.

                    MessageLog();                                           // Constructor, sets some default values.
#ifdef GREAVE_TOLK
    void            add_latest_message(const std::string &msg);             // Adds a message to the latest messages vector.
    void            clear_latest_messages();                                // Clears the latest messages vector.
#endif
    void            load(std::shared_ptr<SQLite::Database> save_db);        // Loads the message log from disk.
    void            msg(std::string str);                                   // Adds a message to the log.
    std::string     render_message_log(bool accept_blank_input = false);    // Renders the message log, returns user input.
    void            save(std::shared_ptr<SQLite::Database> save_db);        // Saves the message log to disk.

private:
    void            clear_messages();                       // Clears the message log.
    void            recalc_window_sizes();                  // Recalculates the size and coordinates of the windows.
    void            reprocess_output();                     // Reprocesses the raw output to fit into the message window.
    void            scroll_to_pixel(int pixel_y);           // Scrolls the scrollbar to the given position.

    bool                        dragging_scrollbar_;        // Is the player currently dragging the scrollbar?
    int                         dragging_scrollbar_offset_; // Used to calculate movement when dragging the scrollbar.
    std::vector<std::string>    output_processed_;          // Processed messages, word-wrapped to fit on the screen.
    std::vector<std::string>    output_raw_;                // Unprocessed messages, which have not yet been word-wrapped to fit on the screen.
    std::string                 input_buffer_;              // The input buffer, where the player enters commands.
    unsigned int                input_window_width_;        // The width of the input window.
    unsigned int                input_window_x_;            // The X coordinate of the input window.
    unsigned int                input_window_y_;            // The Y coordinate of the input window.
    std::string                 last_input_;                // The last input entered by the player.
    int                         offset_;                    // Used for scrolling the text in the output window.
    unsigned int                output_window_height_;      // The height of the output window.
    unsigned int                output_window_width_;       // The width of the output window.
    unsigned int                output_window_x_;           // The X coordinate of the output window.
    unsigned int                output_window_y_;           // The Y coordinate of the output window.

#ifdef GREAVE_TOLK
    std::vector<std::string>    latest_messages_;           // The last messages received after player input; can be repeated if using a screen-reader.
#endif
};

#endif  // GREAVE_CORE_MESSAGE_H_
