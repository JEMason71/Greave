// core/message.hpp -- The main interface to the game, a scrolling message log and an input window.
// Copyright (c) 2019-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.
// Originally based on BearLibTerminal sample code, (c) 2014 Cfyz.

#pragma once
#include "core/greave.hpp"

namespace SQLite { class Database; }    // defined in 3rdparty/SQLiteCpp/Database.h


class MessageLog
{
public:
    static const std::string    SQL_MSGLOG; // SQL strings to construct database tables.

                MessageLog();                                           // Constructor, sets some default values.
    void        load(std::shared_ptr<SQLite::Database> save_db);        // Loads the message log from disk.
    void        msg(std::string str);                                   // Adds a message to the log.
    std::string render_message_log(bool accept_blank_input = false);    // Renders the message log, returns user input.
    void        save(std::shared_ptr<SQLite::Database> save_db);        // Saves the message log to disk.

#ifdef GREAVE_TOLK
    void        add_latest_message(const std::string &msg);             // Adds a message to the latest messages vector.
    void        clear_latest_messages();                                // Clears the latest messages vector.
#endif

private:
    void    clear_messages();               // Clears the message log.
    void    recalc_window_sizes();          // Recalculates the size and coordinates of the windows.
    void    reprocess_output();             // Reprocesses the raw output to fit into the message window.
    void    scroll_to_pixel(int pixel_y);   // Scrolls the scrollbar to the given position.

    bool            m_dragging_scrollbar;               // Is the player currently dragging the scrollbar?
    int             m_dragging_scrollbar_offset;        // Used to calculate movement when dragging the scrollbar.
    std::vector<std::string>    m_output_processed;     // Processed messages, word-wrapped to fit on the screen.
    std::vector<std::string>    m_output_raw;           // Unprocessed messages, which have not yet been word-wrapped to fit on the screen.
    std::string     m_input_buffer;                     // The input buffer, where the player enters commands.
    unsigned int    m_input_window_width;               // The width of the input window.
    unsigned int    m_input_window_x, m_input_window_y; // The X,Y coordinates of the input window.
    std::string     m_last_input;                       // The last input entered by the player.
    int             m_offset;                           // Used for scrolling the text in the output window.
    unsigned int    m_output_window_height, m_output_window_width;  // The height and width of the output window.
    unsigned int    m_output_window_x, m_output_window_y;           // The X,Y coordinates of the output window.

#ifdef GREAVE_TOLK
    std::vector<std::string>    m_latest_messages;  // The last messages received after player input; can be repeated if using a screen-reader.
#endif
};
