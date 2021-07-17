// core/guru.h -- Guru Meditation error-handling and reporting system.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"
#include <exception>
#include <fstream>


class Guru
{
public:
            Guru(std::string log_filename = "");            // Opens the output log for messages.
            ~Guru();                                        // Closes the Guru log file.
    void    cache_nonfatal(bool cache = true);              // Enables or disables cache of nonfatal error messages.
    void    console_ready(bool is_ready = true);            // Tells Guru that we're ready to render Guru error messages on-screen.
    void    dump_nonfatal();                                // Dumps all cached nonfatal messages to the console.
    bool    is_dead() const;                                // Checks if the system has halted.
    void    halt(const std::string &error);                 // Stops the game and displays an error messge.
    void    halt(std::exception &e);                        // As above, but with an exception instead of a string.
    void    intercept_signal(int sig);                      // Catches a segfault or other fatal signal.
    void    log(std::string msg, int type = Guru::INFO);    // Logs a message in the system log file.
    void    nonfatal(std::string error, int type);          // Reports a non-fatal error, which will be logged but will not halt execution unless it cascades.

    static const int    INFO;               // General logging information.
    static const int    WARN;               // Warnings, non-fatal stuff.
    static const int    ERROR, GURU_ERROR;  // Serious errors. Shit is going down.
    static const int    CRITICAL;           // Critical system failure.

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

    static const int    CASCADE_THRESHOLD;          // The amount cascade_count can reach within CASCADE_TIMEOUT seconds before it triggers an abort screen.
    static const int    CASCADE_TIMEOUT;            // The number of seconds without an error to reset the cascade timer.
    static const int    CASCADE_WEIGHT_CRITICAL;    // The amount a critical type log entry will add to the cascade timer.
    static const int    CASCADE_WEIGHT_ERROR;       // The amount an error type log entry will add to the cascade timer.
    static const int    CASCADE_WEIGHT_WARNING;     // The amount a warning type log entry will add to the cascade timer.
    static const std::string    FILENAME_LOG;       // The default name of the log file. Another filename can be specified with open_syslog().
};
