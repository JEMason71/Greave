// core/message.cc -- The main interface to the game, a scrolling message log and an input window.
// Copyright (c) 2019-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.
// Originally based on BearLibTerminal sample code, (c) 2014 Cfyz.

#include "core/message.h"

#include <cmath>

#include <regex>

#ifdef GREAVE_TARGET_WINDOWS
#include "3rdparty/Tolk/Tolk.h"
#endif
#include "core/strx.h"


namespace greave {

// SQL table construction strings.
const std::string MessageLog::SQL_MSGLOG =  "CREATE TABLE 'msglog' ( line INTEGER PRIMARY KEY, text TEXT NOT NULL )";


// Constructor, sets some default values.
MessageLog::MessageLog() : m_dragging_scrollbar(false), m_dragging_scrollbar_offset(0), m_offset(0) { recalc_window_sizes(); }

#ifdef GREAVE_TOLK
// Adds a message to the latest messages vector.
void MessageLog::add_latest_message(const std::string &msg) { m_latest_messages.push_back(msg); }

// Clears the latest messages vector.
void MessageLog::clear_latest_messages() { m_latest_messages.clear(); }
#endif

// Clears the message log.
void MessageLog::clear_messages()
{
    m_output_raw.clear();
    m_output_processed.clear();
    m_input_buffer.clear();
#ifdef GREAVE_TOLK
    m_latest_messages.clear();
#endif
}

// Loads the message log from disk.
void MessageLog::load(std::shared_ptr<SQLite::Database> save_db)
{
    clear_messages();
    m_last_input.clear();
    SQLite::Statement query(*save_db, "SELECT text FROM msglog ORDER BY line ASC");
    while (query.executeStep())
        m_output_raw.push_back(query.getColumn("text").getString());

    reprocess_output();
    m_offset = static_cast<int>(m_output_processed.size() - m_output_window_height);    // Move the offset back to the bottom of the message log.
    m_dragging_scrollbar = false;
    m_dragging_scrollbar_offset = 0;
}

// Adds a message to the log.
void MessageLog::msg(std::string str)
{
    m_output_raw.push_back(str);
    reprocess_output();
    m_offset = m_output_processed.size() - m_output_window_height;
    m_dragging_scrollbar = false;
}

// Recalculates the size and coordinates of the windows.
void MessageLog::recalc_window_sizes()
{
    const std::shared_ptr<Prefs> prefs = core()->prefs();
    const int padding_top = prefs->log_padding_top, padding_bottom = prefs->log_padding_bottom, padding_left = prefs->log_padding_left, padding_right = prefs->log_padding_right;
    int screen_width, screen_height;
    core()->terminal()->get_size(&screen_width, &screen_height);
    m_output_window_width = screen_width - padding_left - padding_right;
    m_output_window_height = screen_height - padding_top - padding_bottom;
    m_input_window_width = screen_width - padding_left - padding_right;
    m_output_window_x = m_input_window_x = padding_left;
    m_output_window_y = padding_top;
    m_input_window_y = screen_height - padding_bottom + 1;
}

// Renders the message log, returns user input.
std::string MessageLog::render_message_log(bool accept_blank_input)
{
    const auto prefs = core()->prefs();

    auto coloured_value_indicator = [](const std::string &name, int current, int max, char colour_ch) -> std::string {
        std::string colour = "{" + std::string(1, colour_ch) + "}", colour_dark = "{" + std::string(1, colour_ch + 32) + "}";
        return colour + std::to_string(current) + colour_dark + "/" + colour + std::to_string(max) + colour_dark + name;
    };

    std::string status_str;
    if (core()->world())
    {
        std::string stance_str;
        const auto player = core()->world()->player();
        switch (player->stance())
        {
            case CombatStance::AGGRESSIVE: stance_str = "{R}a"; break;
            case CombatStance::BALANCED: stance_str = "{G}b"; break;
            case CombatStance::DEFENSIVE: stance_str = "{U}d"; break;
        }
        if (player->has_buff(Buff::Type::CAREFUL_AIM)) stance_str += "{W}:{G}ca";
        if (player->has_buff(Buff::Type::EYE_FOR_AN_EYE)) stance_str += "{W}:{R}ef";
        if (player->has_buff(Buff::Type::GRIT)) stance_str += "{W}:{U}gr";
        if (player->has_buff(Buff::Type::QUICK_ROLL)) stance_str += "{W}:{U}qr";
        if (player->has_buff(Buff::Type::SHIELD_WALL)) stance_str += "{W}:{U}sh";
        status_str = "{W}<" + stance_str + "{W}:" + coloured_value_indicator("hp", player->hp(), player->hp(true), 'R');
        if (player->sp() < player->sp(true)) status_str += "{W}:" + coloured_value_indicator("sp", player->sp(), player->sp(true), 'G');
        if (player->mp() < player->mp(true)) status_str += "{W}:" + coloured_value_indicator("mp", player->mp(), player->mp(true), 'U');
        status_str += "{W}>";
        core()->screen_read(status_str, false);
    }

    while(true)
    {
        // Clear the screen, fill in dark gray areas for the input and output areas.
        core()->terminal()->cls();
        core()->terminal()->fill(m_output_window_x, m_output_window_y, m_output_window_width, m_output_window_height, Terminal::Colour::DARKEST_GREY);
        core()->terminal()->fill(m_input_window_x, m_input_window_y, m_input_window_width, 1, Terminal::Colour::DARKEST_GREY);

        // Render the visible part of the output window.
        int start = m_offset, end = m_output_processed.size();
        if (end - start > static_cast<int>(m_output_window_height)) end = m_output_window_height + start;
        for (int i = start; i < end; i++)
        {
            if (i < 0 || i >= static_cast<int>(m_output_processed.size())) continue;
            core()->terminal()->print(m_output_processed.at(i), m_output_window_x, m_output_window_y + i - start);
        }

        // Render the input buffer.
        std::string input_buf = "{W}" + m_input_buffer;
        if (core()->world()) input_buf = status_str + " " + input_buf;
        const unsigned int input_buf_len = StrX::strlen_colour(input_buf);
        if (input_buf_len > m_input_window_width) input_buf = input_buf.substr(0, m_input_window_width);
        core()->terminal()->print(input_buf, m_input_window_x, m_input_window_y);

        // Render the scroll bar.
        const int scrollbar_x = prefs->log_padding_left + m_output_window_width;
        const int scrollbar_height = std::min<int>(std::ceil(m_output_window_height * (m_output_window_height / static_cast<float>(m_output_processed.size()))), m_output_window_height);
        int scrollbar_offset;
        if (!(m_output_processed.size() - m_output_window_height)) scrollbar_offset = (prefs->log_padding_top + (m_output_window_height - scrollbar_height));
        else scrollbar_offset = (prefs->log_padding_top + (m_output_window_height - scrollbar_height) * (static_cast<float>(m_offset) / static_cast<float>(m_output_processed.size() - m_output_window_height)));
        for (unsigned int i = 0; i < m_output_window_height; i++)
            core()->terminal()->put('|', scrollbar_x, prefs->log_padding_top + i, Terminal::Colour::WHITE);
        for (int i = 0; i < scrollbar_height; i++)
            core()->terminal()->put(' ', scrollbar_x, i + scrollbar_offset, Terminal::Colour::WHITE_BG);

        // Render the cursor on the input buffer.
        if (input_buf_len < m_input_window_width)
        {
            core()->terminal()->cursor(true);
            core()->terminal()->move_cursor(m_input_window_x + input_buf_len, m_input_window_y);
        }
        else core()->terminal()->cursor(false);

        core()->terminal()->refresh();

        const int key = core()->terminal()->get_key();
        const bool is_dead = core()->guru()->is_dead();
        const int scroll_bottom = static_cast<int>(m_output_processed.size() - m_output_window_height);
        if (key == Terminal::Key::CLOSE)
        {
            core()->cleanup();
            if (is_dead) exit(EXIT_FAILURE);
            else exit(EXIT_SUCCESS);
        }
        else if (key == Terminal::Key::RESIZED)
        {
            reprocess_output();
            m_offset = m_output_processed.size() - m_output_window_height;
        }
        else if (key >= ' ' && key <= '~' && key != '{' && key != '}') m_input_buffer += static_cast<char>(key);
        else if (key == Terminal::Key::BACKSPACE && m_input_buffer.size()) m_input_buffer.pop_back();
        else if ((key == Terminal::Key::CR || key == Terminal::Key::LF) && (m_input_buffer.size() || accept_blank_input))
        {
            std::string result = std::regex_replace(m_input_buffer, std::regex("^ +| +$|( ) +"), "$1");
            if (result.size())
            {
                core()->message("{c}> " + result, true);
                m_input_buffer = "";
                m_last_input = result;
                return result;
            }
            else
            {
                m_input_buffer = "";
                if (accept_blank_input)
                {
                    m_last_input = "";
                    return "";
                }
            }
        }
        else if ((key == Terminal::Key::ARROW_UP || key == Terminal::Key::MOUSE_SCROLL_UP) && m_offset > 1)
        {
            m_offset -= (key == Terminal::Key::MOUSE_SCROLL_UP ? prefs->log_mouse_scroll_step : 1);
            if (m_offset < 1) m_offset = 1;
        }
        else if ((key == Terminal::Key::ARROW_DOWN || key == Terminal::Key::MOUSE_SCROLL_DOWN) && m_offset < scroll_bottom)
        {
            m_offset += (key == Terminal::Key::MOUSE_SCROLL_DOWN ? prefs->log_mouse_scroll_step : 1);
            if (m_offset > scroll_bottom) m_offset = scroll_bottom;
        }
        else if (key == Terminal::Key::HOME && m_output_processed.size() > m_output_window_height) m_offset = 1;
        else if (key == Terminal::Key::END) m_offset = scroll_bottom;
        else if (key == Terminal::Key::PAGE_UP && m_output_processed.size() > m_output_window_height)
        {
            m_offset -= m_output_window_height;
            if (m_offset < 1) m_offset = 1;
        }
        else if (key == Terminal::Key::PAGE_DOWN)
        {
            m_offset += m_output_window_height;
            if (m_offset > scroll_bottom) m_offset = scroll_bottom;
        }
        else if (key == Terminal::Key::MOUSE_LEFT && core()->terminal()->get_mouse_x() == scrollbar_x && m_output_processed.size() > m_output_window_height)
        {
            const int pixel_y = core()->terminal()->get_mouse_y_pixel();
            if (pixel_y >= scrollbar_offset * core()->terminal()->cell_height() && pixel_y <= (scrollbar_offset + scrollbar_height) * core()->terminal()->cell_height())
            {
                // Clicked on the scrollbar handle: start dragging.
                m_dragging_scrollbar = true;
                m_dragging_scrollbar_offset = pixel_y - (scrollbar_offset * core()->terminal()->cell_height());
            }
            else scroll_to_pixel(core()->terminal()->get_mouse_y_pixel() - scrollbar_height * core()->terminal()->cell_height() / 2);
        }
        else if (key == Terminal::Key::MOUSE_LEFT_RELEASED) m_dragging_scrollbar = false;
        else if (key == Terminal::Key::MOUSE_HAS_MOVED && m_dragging_scrollbar) scroll_to_pixel(core()->terminal()->get_mouse_y_pixel() - m_dragging_scrollbar_offset);
#ifdef GREAVE_TOLK
        else if (key == Terminal::Key::TAB && m_latest_messages.size() && (prefs->screen_reader_external || prefs->screen_reader_sapi))
        {
            Tolk_Silence();
            for (auto line : m_latest_messages)
            {
                const std::wstring wide_line(line.begin(), line.end());
                Tolk_Output(wide_line.c_str());
            }
        }
#endif
    }
    return "";
}

// Reprocesses the raw output to fit into the message window.
void MessageLog::reprocess_output()
{
    recalc_window_sizes();
    while (m_output_raw.size() > static_cast<unsigned int>(core()->prefs()->log_max_size))
        m_output_raw.erase(m_output_raw.begin());

    m_output_processed.clear();
    for (auto line : m_output_raw)
    {
        bool same_line = false;
        if (line.size() >= 3 && line.substr(0, 3) == "{0}")
        {
            line = line.substr(3);
            same_line = true;
        }
        std::vector<std::string> split_line = StrX::string_explode_colour(line, m_output_window_width);
        if (!same_line) m_output_processed.push_back("");
        m_output_processed.insert(m_output_processed.end(), split_line.begin(), split_line.end());
    }
}

// Saves the message log to disk.
void MessageLog::save(std::shared_ptr<SQLite::Database> save_db)
{
    for (unsigned int i = 0; i < m_output_raw.size(); i++)
    {
        SQLite::Statement query(*save_db, "INSERT INTO msglog ( line, text ) VALUES ( :line, :text )");
        query.bind(":line", i);
        query.bind(":text", m_output_raw.at(i));
        query.exec();
    }
}

// Scrolls the scrollbar to the given position.
void MessageLog::scroll_to_pixel(int pixel_y)
{
    pixel_y -= core()->prefs()->log_padding_top * core()->terminal()->cell_height();
    const float factor = pixel_y / (static_cast<float>(m_output_window_height) * core()->terminal()->cell_height());
    m_offset = std::max<int>(1, std::min<int>(m_output_processed.size() - m_output_window_height, m_output_processed.size() * factor));
}

}   // namespace greave
