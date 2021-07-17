// actions/status.h -- Meta status actions, such as checking the player's score or condition.
/// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_ACTIONS_STATUS_H_
#define GREAVE_ACTIONS_STATUS_H_

#include <string>


namespace greave {

class ActionStatus
{
public:
    static void         score();        // Check the player's current total score.
    static void         skills();       // Checks the player's skill levels.
    static void         status();       // Checks the player's overall status.
    static std::string  temperature(bool print = true); // Displays the current temperature.
    static std::string  time(bool print = true);        // Determines the current time of day.
    static void         weather();      // Checks the nearby weather.
};

}       // namespace greave
#endif  // GREAVE_ACTIONS_STATUS_H_
