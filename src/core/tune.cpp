// core/tune.cpp -- The Tune class loads data from tune.yml, allowing for various numbers to be tweaked on-the-fly.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/yaml-cpp/yaml.h"
#include "uni/uni-core.hpp"


// Constructor, loads data from tune.yml
Tune::Tune()
{
    const std::shared_ptr<Guru> guru = core()->guru();

    guru->log("Loading tuning data from tune.yml...");
    try
    {
        const YAML::Node yaml_tune = YAML::LoadFile("data/tune.yml");
        YAML::Node yaml_override;
        bool override = false;
        if (Util::file_exists("userdata/tune.yml"))
        {
            guru->log("User override tune.yml detected, loading user settings...");
            yaml_override = YAML::LoadFile("userdata/tune.yml");
            override = true;
        }

        /*
        auto get_tune = [&yaml_tune, &yaml_override, &override, &guru](const std::string &value) -> double
        {
            if (override && yaml_override[value]) return yaml_override[value].as<double>();
            else if (!yaml_tune[value]) guru->halt("Missing or incorrect value in tune.yml: " + value);
            return yaml_tune[value].as<double>();
        };
        */

        auto get_tune_bool = [&yaml_tune, &yaml_override, &override, &guru](const std::string &value) -> bool
        {
            if (override && yaml_override[value]) return yaml_override[value].as<bool>();
            else if (!yaml_tune[value]) guru->halt("Missing or incorrect boolean value in tune.yml: " + value);
            return yaml_tune[value].as<bool>();
        };

        /*
        auto get_tune_tuple = [&yaml_tune, &yaml_override, &override, &guru](const std::string &value, float *result, unsigned int count) -> void
        {
            YAML::Node node = yaml_tune;
            if (override && yaml_override[value]) node = yaml_override;
            if (!node[value] || !node[value].IsSequence() || node[value].size() != count) guru->halt("Missing or incorrect value in tune.yml: " + value);
            for (unsigned int i = 0; i < count; i++)
                result[i] = node[value][i].as<double>();
        };
        */

        /*
        auto get_tune_tuple_int = [&yaml_tune, &yaml_override, &override, &guru](const std::string &value, int *result, unsigned int count) -> void
        {
            YAML::Node node = yaml_tune;
            if (override && yaml_override[value]) node = yaml_override;
            if (!node[value] || !node[value].IsSequence() || node[value].size() != count) guru->halt("Missing or incorrect value in tune.yml: " + value);
            for (unsigned int i = 0; i < count; i++)
                result[i] = node[value][i].as<int>();
        };
        */

        /*
        auto get_tune_string = [&yaml_tune, &yaml_override, &override, &guru](const std::string &value) -> std::string
        {
            if (override && yaml_override[value]) return yaml_override[value].as<std::string>();
            else if (!yaml_tune[value]) guru->halt("Missing or incorrect string value in tune.yml: " + value);
            return yaml_tune[value].as<std::string>();
        };
        */

#ifdef GREAVE_TOLK
        screen_reader_external = get_tune_bool("screen_reader_external");
        screen_reader_process_square_brackets = get_tune_bool("screen_reader_process_square_brackets");
        screen_reader_sapi = get_tune_bool("screen_reader_sapi");
#endif
    }
    catch (std::exception &e)
    {
        guru->halt(e);
    }
}
