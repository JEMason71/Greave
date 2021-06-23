// core/tune.cpp -- The Tune class loads data from tune.yml, allowing for various numbers to be tweaked on-the-fly.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/yaml-cpp/yaml.h"
#include "core/core.hpp"
#include "core/filex.hpp"
#include "core/guru.hpp"
#include "core/tune.hpp"


// Constructor, loads data from tune.yml
Tune::Tune()
{
    const std::shared_ptr<Guru> guru = core()->guru();

    guru->log("Loading tuning data from tune.yml...");
    const YAML::Node yaml_tune = YAML::LoadFile("data/tune.yml");
    YAML::Node yaml_override;
    bool override = false;
    if (FileX::file_exists("userdata/tune.yml"))
    {
        guru->log("User override tune.yml detected, loading user settings...");
        yaml_override = YAML::LoadFile("userdata/tune.yml");
        override = true;
    }

    auto get_tune = [&yaml_tune, &yaml_override, &override](const std::string &value) -> double
    {
        if (override && yaml_override[value]) return yaml_override[value].as<double>();
        else if (!yaml_tune[value]) throw std::runtime_error("Missing or incorrect value in tune.yml: " + value);
        return yaml_tune[value].as<double>();
    };

    auto get_tune_bool = [&yaml_tune, &yaml_override, &override](const std::string &value) -> bool
    {
        if (override && yaml_override[value]) return yaml_override[value].as<bool>();
        else if (!yaml_tune[value]) throw std::runtime_error("Missing or incorrect boolean value in tune.yml: " + value);
        return yaml_tune[value].as<bool>();
    };

    /*
    auto get_tune_tuple = [&yaml_tune, &yaml_override, &override](const std::string &value, float *result, unsigned int count) -> void
    {
        YAML::Node node = yaml_tune;
        if (override && yaml_override[value]) node = yaml_override;
        if (!node[value] || !node[value].IsSequence() || node[value].size() != count) throw std::runtime_error("Missing or incorrect value in tune.yml: " + value);
        for (unsigned int i = 0; i < count; i++)
            result[i] = node[value][i].as<double>();
    };
    */

    /*
    auto get_tune_tuple_int = [&yaml_tune, &yaml_override, &override](const std::string &value, int *result, unsigned int count) -> void
    {
        YAML::Node node = yaml_tune;
        if (override && yaml_override[value]) node = yaml_override;
        if (!node[value] || !node[value].IsSequence() || node[value].size() != count) throw std::runtime_error("Missing or incorrect value in tune.yml: " + value);
        for (unsigned int i = 0; i < count; i++)
            result[i] = node[value][i].as<int>();
    };
    */

    auto get_tune_string = [&yaml_tune, &yaml_override, &override](const std::string &value) -> std::string
    {
        if (override && yaml_override[value]) return yaml_override[value].as<std::string>();
        else if (!yaml_tune[value]) throw std::runtime_error("Missing or incorrect string value in tune.yml: " + value);
        return yaml_tune[value].as<std::string>();
    };

    blt_console_size = get_tune_string("blt_console_size");
    blt_font = get_tune_string("blt_font");
    blt_font_size = get_tune("blt_font_size");
    blt_log_file = get_tune_string("blt_log_file");
    blt_log_level = get_tune_string("blt_log_level");
    blt_vsync = get_tune_bool("blt_vsync");
    colour_black = get_tune_string("colour_black");
    colour_blue = get_tune_string("colour_blue");
    colour_blue_dark = get_tune_string("colour_blue_dark");
    colour_cyan = get_tune_string("colour_cyan");
    colour_cyan_dark = get_tune_string("colour_cyan_dark");
    colour_green = get_tune_string("colour_green");
    colour_green_dark = get_tune_string("colour_green_dark");
    colour_grey = get_tune_string("colour_grey");
    colour_grey_dark = get_tune_string("colour_grey_dark");
    colour_magenta = get_tune_string("colour_magenta");
    colour_magenta_dark = get_tune_string("colour_magenta_dark");
    colour_red = get_tune_string("colour_red");
    colour_red_dark = get_tune_string("colour_red_dark");
    colour_white = get_tune_string("colour_white");
    colour_yellow = get_tune_string("colour_yellow");
    colour_yellow_dark = get_tune_string("colour_yellow_dark");
    curses_custom_colours = get_tune_bool("curses_custom_colours");
    log_max_size = get_tune("log_max_size");
    log_mouse_scroll_step = get_tune("log_mouse_scroll_step");
    log_padding_bottom = get_tune("log_padding_bottom");
    log_padding_left = get_tune("log_padding_left");
    log_padding_right = get_tune("log_padding_right");
    log_padding_top = get_tune("log_padding_top");
    monochrome_mode = get_tune_bool("monochrome_mode");
#ifdef GREAVE_TOLK
    screen_reader_external = get_tune_bool("screen_reader_external");
    screen_reader_process_square_brackets = get_tune_bool("screen_reader_process_square_brackets");
    screen_reader_sapi = get_tune_bool("screen_reader_sapi");
#endif
    terminal = get_tune_string("terminal");
}
