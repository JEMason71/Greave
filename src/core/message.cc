// core/message.cc -- The main interface to the game, a scrolling message log and an input window.
// Copyright (c) 2019-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.
// Originally based on BearLibTerminal sample code, (c) 2014 Cfyz.

#ifdef GREAVE_TARGET_WINDOWS
#include "3rdparty/Tolk/Tolk.h"
#endif
#include "core/core.h"
#include "core/message.h"
#include "core/strx.h"

#include <cmath>
#include <regex>


// SQL string to construct database table.
constexpr char MessageLog::SQL_MSGLOG[] = "CREATE TABLE 'msglog' ( line INTEGER PRIMARY KEY, text TEXT NOT NULL )";


// Constructor, sets some default values.
MessageLog::MessageLog() : dragging_scrollbar_(false), dragging_scrollbar_offset_(0), offset_(0) { recalc_window_sizes(); }

#ifdef GREAVE_TOLK
// Adds a message to the latest messages vector.
void MessageLog::add_latest_message(const std::string &msg) { latest_messages_.push_back(msg); }

// Clears the latest messages vector.
void MessageLog::clear_latest_messages() { latest_messages_.clear(); }
#endif

// Clears the message log.
void MessageLog::clear_messages()
{
    output_raw_.clear();
    output_processed_.clear();
    input_buffer_.clear();
#ifdef GREAVE_TOLK
    latest_messages_.clear();
#endif
}

// Loads the message log from disk.
void MessageLog::load(std::shared_ptr<SQLite::Database> save_db)
{
    clear_messages();
    last_input_.clear();
    SQLite::Statement query(*save_db, "SELECT text FROM msglog ORDER BY line ASC");
    while (query.executeStep())
        output_raw_.push_back(query.getColumn("text").getString());

    reprocess_output();
    offset_ = static_cast<int>(output_processed_.size() - output_window_height_);    // Move the offset back to the bottom of the message log.
    dragging_scrollbar_ = false;
    dragging_scrollbar_offset_ = 0;
}

// Adds a message to the log.
void MessageLog::msg(std::string str)
{
    output_raw_.push_back(str);
    reprocess_output();
    offset_ = output_processed_.size() - output_window_height_;
    dragging_scrollbar_ = false;
}

// Recalculates the size and coordinates of the windows.
void MessageLog::recalc_window_sizes()
{
    const std::shared_ptr<Prefs> prefs = core()->prefs();
    const int padding_top = prefs->log_padding_top, padding_bottom = prefs->log_padding_bottom, padding_left = prefs->log_padding_left, padding_right = prefs->log_padding_right;
    int screen_width, screen_height;
    core()->terminal()->get_size(&screen_width, &screen_height);
    output_window_width_ = screen_width - padding_left - padding_right;
    output_window_height_ = screen_height - padding_top - padding_bottom;
    input_window_width_ = screen_width - padding_left - padding_right;
    output_window_x_ = input_window_x_ = padding_left;
    output_window_y_ = padding_top;
    input_window_y_ = screen_height - padding_bottom + 1;
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
        core()->terminal()->fill(output_window_x_, output_window_y_, output_window_width_, output_window_height_, Terminal::Colour::DARKEST_GREY);
        core()->terminal()->fill(input_window_x_, input_window_y_, input_window_width_, 1, Terminal::Colour::DARKEST_GREY);

        // Render the visible part of the output window.
        int start = offset_, end = output_processed_.size();
        if (end - start > static_cast<int>(output_window_height_)) end = output_window_height_ + start;
        for (int i = start; i < end; i++)
        {
            if (i < 0 || i >= static_cast<int>(output_processed_.size())) continue;
            core()->terminal()->print(output_processed_.at(i), output_window_x_, output_window_y_ + i - start);
        }

        // Render the input buffer.
        std::string input_buf = "{W}" + input_buffer_;
        if (core()->world()) input_buf = status_str + " " + input_buf;
        const unsigned int input_buf_len = StrX::strlen_colour(input_buf);
        if (input_buf_len > input_window_width_) input_buf = input_buf.substr(0, input_window_width_);
        core()->terminal()->print(input_buf, input_window_x_, input_window_y_);

        // Render the scroll bar.
        const int scrollbar_x = prefs->log_padding_left + output_window_width_;
        const int scrollbar_height = std::min<int>(std::ceil(output_window_height_ * (output_window_height_ / static_cast<float>(output_processed_.size()))), output_window_height_);
        int scrollbar_offset;
        if (!(output_processed_.size() - output_window_height_)) scrollbar_offset = (prefs->log_padding_top + (output_window_height_ - scrollbar_height));
        else scrollbar_offset = (prefs->log_padding_top + (output_window_height_ - scrollbar_height) * (static_cast<float>(offset_) / static_cast<float>(output_processed_.size() - output_window_height_)));
        for (unsigned int i = 0; i < output_window_height_; i++)
            core()->terminal()->put('|', scrollbar_x, prefs->log_padding_top + i, Terminal::Colour::WHITE);
        for (int i = 0; i < scrollbar_height; i++)
            core()->terminal()->put(' ', scrollbar_x, i + scrollbar_offset, Terminal::Colour::WHITE_BG);

        // Render the cursor on the input buffer.
        if (input_buf_len < input_window_width_)
        {
            core()->terminal()->cursor(true);
            core()->terminal()->move_cursor(input_window_x_ + input_buf_len, input_window_y_);
        }
        else core()->terminal()->cursor(false);

        core()->terminal()->refresh();

        const int key = core()->terminal()->get_key();
        const bool is_dead = core()->guru()->is_dead();
        const int scroll_bottom = static_cast<int>(output_processed_.size() - output_window_height_);
        if (key == Terminal::Key::CLOSE)
        {
            core()->cleanup();
            if (is_dead) exit(EXIT_FAILURE);
            else exit(EXIT_SUCCESS);
        }
        else if (key == Terminal::Key::RESIZED)
        {
            reprocess_output();
            offset_ = output_processed_.size() - output_window_height_;
        }
        else if (key >= ' ' && key <= '~' && key != '{' && key != '}') input_buffer_ += static_cast<char>(key);
        else if (key == Terminal::Key::BACKSPACE && input_buffer_.size()) input_buffer_.pop_back();
        else if ((key == Terminal::Key::CR || key == Terminal::Key::LF) && (input_buffer_.size() || accept_blank_input))
        {
            std::string result = std::regex_replace(input_buffer_, std::regex("^ +| +$|( ) +"), "$1");
            if (result.size())
            {
                core()->message("{c}> " + result, true);
                input_buffer_ = "";
                last_input_ = result;
                return result;
            }
            else
            {
                input_buffer_ = "";
                if (accept_blank_input)
                {
                    last_input_ = "";
                    return "";
                }
            }
        }
        else if ((key == Terminal::Key::ARROW_UP || key == Terminal::Key::MOUSE_SCROLL_UP) && offset_ > 1)
        {
            offset_ -= (key == Terminal::Key::MOUSE_SCROLL_UP ? prefs->log_mouse_scroll_step : 1);
            if (offset_ < 1) offset_ = 1;
        }
        else if ((key == Terminal::Key::ARROW_DOWN || key == Terminal::Key::MOUSE_SCROLL_DOWN) && offset_ < scroll_bottom)
        {
            offset_ += (key == Terminal::Key::MOUSE_SCROLL_DOWN ? prefs->log_mouse_scroll_step : 1);
            if (offset_ > scroll_bottom) offset_ = scroll_bottom;
        }
        else if (key == Terminal::Key::HOME && output_processed_.size() > output_window_height_) offset_ = 1;
        else if (key == Terminal::Key::END) offset_ = scroll_bottom;
        else if (key == Terminal::Key::PAGE_UP && output_processed_.size() > output_window_height_)
        {
            offset_ -= output_window_height_;
            if (offset_ < 1) offset_ = 1;
        }
        else if (key == Terminal::Key::PAGE_DOWN)
        {
            offset_ += output_window_height_;
            if (offset_ > scroll_bottom) offset_ = scroll_bottom;
        }
        else if (key == Terminal::Key::MOUSE_LEFT && core()->terminal()->get_mouse_x() == scrollbar_x && output_processed_.size() > output_window_height_)
        {
            const int pixel_y = core()->terminal()->get_mouse_y_pixel();
            if (pixel_y >= scrollbar_offset * core()->terminal()->cell_height() && pixel_y <= (scrollbar_offset + scrollbar_height) * core()->terminal()->cell_height())
            {
                // Clicked on the scrollbar handle: start dragging.
                dragging_scrollbar_ = true;
                dragging_scrollbar_offset_ = pixel_y - (scrollbar_offset * core()->terminal()->cell_height());
            }
            else scroll_to_pixel(core()->terminal()->get_mouse_y_pixel() - scrollbar_height * core()->terminal()->cell_height() / 2);
        }
        else if (key == Terminal::Key::MOUSE_LEFT_RELEASED) dragging_scrollbar_ = false;
        else if (key == Terminal::Key::MOUSE_HAS_MOVED && dragging_scrollbar_) scroll_to_pixel(core()->terminal()->get_mouse_y_pixel() - dragging_scrollbar_offset_);
#ifdef GREAVE_TOLK
        else if (key == Terminal::Key::TAB && latest_messages_.size() && (prefs->screen_reader_external || prefs->screen_reader_sapi))
        {
            Tolk_Silence();
            for (auto line : latest_messages_)
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
    while (output_raw_.size() > static_cast<unsigned int>(core()->prefs()->log_max_size))
        output_raw_.erase(output_raw_.begin());

    output_processed_.clear();
    for (auto line : output_raw_)
    {
        bool same_line = false;
        if (line.size() >= 3 && line.substr(0, 3) == "{0}")
        {
            line = line.substr(3);
            same_line = true;
        }
        std::vector<std::string> split_line = StrX::string_explode_colour(line, output_window_width_);
        if (!same_line) output_processed_.push_back("");
        output_processed_.insert(output_processed_.end(), split_line.begin(), split_line.end());
    }
}

// Saves the message log to disk.
void MessageLog::save(std::shared_ptr<SQLite::Database> save_db)
{
    for (unsigned int i = 0; i < output_raw_.size(); i++)
    {
        SQLite::Statement query(*save_db, "INSERT INTO msglog ( line, text ) VALUES ( :line, :text )");
        query.bind(":line", i);
        query.bind(":text", output_raw_.at(i));
        query.exec();
    }
}

// Scrolls the scrollbar to the given position.
void MessageLog::scroll_to_pixel(int pixel_y)
{
    pixel_y -= core()->prefs()->log_padding_top * core()->terminal()->cell_height();
    const float factor = pixel_y / (static_cast<float>(output_window_height_) * core()->terminal()->cell_height());
    offset_ = std::max<int>(1, std::min<int>(output_processed_.size() - output_window_height_, output_processed_.size() * factor));
}
