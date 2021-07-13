// core/prefs.cpp -- The Prefs class loads data from prefs.yml, allowing the user to configure game settings.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/yaml-cpp/yaml.h"
#include "core/core.hpp"
#include "core/filex.hpp"
#include "core/guru.hpp"
#include "core/prefs.hpp"


// Constructor, loads data from prefs.yml
Prefs::Prefs()
{
    const std::shared_ptr<Guru> guru = core()->guru();
    try
    {
        guru->log("Loading user preferences from prefs.yml...");
        const YAML::Node yaml_pref = YAML::LoadFile("data/misc/prefs.yml");
        YAML::Node yaml_override;
        bool override = false;
        if (FileX::file_exists("userdata/prefs.yml"))
        {
            guru->log("User override prefs.yml detected, loading user settings...");
            yaml_override = YAML::LoadFile("userdata/prefs.yml");
            override = true;
        }

        auto get_pref = [&yaml_pref, &yaml_override, &override](const std::string &value) -> double
        {
            if (override && yaml_override[value]) return yaml_override[value].as<double>();
            else if (!yaml_pref[value]) throw std::runtime_error("Missing or incorrect value in prefs.yml: " + value);
            return yaml_pref[value].as<double>();
        };

        auto get_pref_bool = [&yaml_pref, &yaml_override, &override](const std::string &value) -> bool
        {
            if (override && yaml_override[value]) return yaml_override[value].as<bool>();
            else if (!yaml_pref[value]) throw std::runtime_error("Missing or incorrect boolean value in prefs.yml: " + value);
            return yaml_pref[value].as<bool>();
        };

        auto get_pref_string = [&yaml_pref, &yaml_override, &override](const std::string &value) -> std::string
        {
            if (override && yaml_override[value]) return yaml_override[value].as<std::string>();
            else if (!yaml_pref[value]) throw std::runtime_error("Missing or incorrect string value in prefs.yml: " + value);
            return yaml_pref[value].as<std::string>();
        };

        colour_black = get_pref_string("colour_black");
        colour_blue = get_pref_string("colour_blue");
        colour_blue_dark = get_pref_string("colour_blue_dark");
        colour_cyan = get_pref_string("colour_cyan");
        colour_cyan_dark = get_pref_string("colour_cyan_dark");
        colour_green = get_pref_string("colour_green");
        colour_green_dark = get_pref_string("colour_green_dark");
        colour_grey = get_pref_string("colour_grey");
        colour_grey_dark = get_pref_string("colour_grey_dark");
        colour_grey_darkest = get_pref_string("colour_grey_darkest");
        colour_magenta = get_pref_string("colour_magenta");
        colour_magenta_dark = get_pref_string("colour_magenta_dark");
        colour_red = get_pref_string("colour_red");
        colour_red_dark = get_pref_string("colour_red_dark");
        colour_white = get_pref_string("colour_white");
        colour_yellow = get_pref_string("colour_yellow");
        colour_yellow_dark = get_pref_string("colour_yellow_dark");
        curses_custom_colours = get_pref_bool("curses_custom_colours");
        log_max_size = get_pref("log_max_size");
        log_mouse_scroll_step = get_pref("log_mouse_scroll_step");
        log_padding_bottom = get_pref("log_padding_bottom");
        log_padding_left = get_pref("log_padding_left");
        log_padding_right = get_pref("log_padding_right");
        log_padding_top = get_pref("log_padding_top");
        monochrome_mode = get_pref_bool("monochrome_mode");
        save_file_slots = get_pref("save_file_slots");
    #ifdef GREAVE_TOLK
        screen_reader_external = get_pref_bool("screen_reader_external");
        screen_reader_process_square_brackets = get_pref_bool("screen_reader_process_square_brackets");
        screen_reader_sapi = get_pref_bool("screen_reader_sapi");
    #endif
        sdl_console_size = get_pref_string("sdl_console_size");
        sdl_font = get_pref_string("sdl_font");
        sdl_font_size = get_pref("sdl_font_size");
        sdl_vsync = get_pref_bool("sdl_vsync");
        terminal = get_pref_string("terminal");
    }
    catch (std::exception& e)
    {
        throw std::runtime_error("Error while loading prefs.yml: " + std::string(e.what()));
    }
}
