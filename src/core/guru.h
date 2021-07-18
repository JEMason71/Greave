// core/guru.h -- Guru Meditation error-handling and reporting system.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#ifndef GREAVE_CORE_GURU_H_
#define GREAVE_CORE_GURU_H_

#include <exception>
#include <fstream>
#include <string>
#include <vector>


class Guru
{
public:
    static constexpr int    GURU_INFO =     0;  // General logging information.
    static constexpr int    GURU_WARN =     1;  // Warnings, non-fatal stuff.
    static constexpr int    GURU_ERROR =    2;  // Serious errors. Shit is going down.
    static constexpr int    GURU_CRITICAL = 3;  // Critical system failure.

            Guru(std::string log_filename = "");            // Opens the output log for messages.
            ~Guru();                                        // Closes the Guru log file.
    void    cache_nonfatal(bool cache = true);              // Enables or disables cache of nonfatal error messages.
    void    console_ready(bool is_ready = true);            // Tells Guru that we're ready to render Guru error messages on-screen.
    void    dump_nonfatal();                                // Dumps all cached nonfatal messages to the console.
    bool    is_dead() const;                                // Checks if the system has halted.
    void    halt(const std::string &error);                 // Stops the game and displays an error messge.
    void    halt(std::exception &e);                        // As above, but with an exception instead of a string.
    void    intercept_signal(int sig);                      // Catches a segfault or other fatal signal.
    void    log(std::string msg, int type = Guru::GURU_INFO);    // Logs a message in the system log file.
    void    nonfatal(std::string error, int type);          // Reports a non-fatal error, which will be logged but will not halt execution unless it cascades.

private:
    bool            m_cache_nonfatal;   // Temporarily caches nonfatal error messages.
    int             m_cascade_count;    // Keeps track of rapidly-occurring, non-fatal error messages.
    bool            m_cascade_failure;  // Is a cascade failure in progress?
    time_t          m_cascade_timer;    // Timer to check the speed of non-halting Guru warnings, to prevent cascade locks.
    bool            m_console_ready;    // Have we fully initialized the console yet?
    bool            m_dead_already;     // Have we already died? Is this crash within the Guru subsystem?
    std::string     m_last_log_message; // Records the last log message, to avoid spamming the log with repeats.
    std::vector<std::string>    m_nonfatal_cache;   // Cache of nonfatal error messages.
    std::ofstream   m_syslog;           // The system log file.

    static constexpr int    CASCADE_THRESHOLD =         25; // The amount cascade_count can reach within CASCADE_TIMEOUT seconds before it triggers an abort screen.
    static constexpr int    CASCADE_TIMEOUT =           30; // The number of seconds without an error to reset the cascade timer.
    static constexpr int    CASCADE_WEIGHT_CRITICAL =   20; // The amount a critical type log entry will add to the cascade timer.
    static constexpr int    CASCADE_WEIGHT_ERROR =      5;  // The amount an error type log entry will add to the cascade timer.
    static constexpr int    CASCADE_WEIGHT_WARNING =    1;  // The amount a warning type log entry will add to the cascade timer.
    static const char       FILENAME_LOG[];                 // The default name of the log file. Another filename can be specified with open_syslog().
};

#endif  // GREAVE_CORE_GURU_H_
