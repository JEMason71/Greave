// core/guru.cpp -- Guru Meditation error-handling and reporting system.
// Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "core/core.hpp"
#include "core/filex.hpp"
#include "core/guru.hpp"
#include "core/message.hpp"
#include "core/terminal.hpp"

#include <csignal>
#include <ctime>
#include <sstream>


const int   Guru::INFO =        0;  // General logging information.
const int   Guru::WARN =        1;  // Warnings, non-fatal stuff.
const int   Guru::ERROR =       2;  // Serious errors. Shit is going down.
const int   Guru::CRITICAL =    3;  // Critical system failure.

const int   Guru::CASCADE_THRESHOLD =       20; // The amount m_cascade_count can reach within CASCADE_TIMEOUT seconds before it triggers an abort screen.
const int   Guru::CASCADE_TIMEOUT =         30; // The number of seconds without an error to reset the cascade timer.
const int   Guru::CASCADE_WEIGHT_CRITICAL = 4;  // The amount a critical type log entry will add to the cascade timer.
const int   Guru::CASCADE_WEIGHT_ERROR =    2;  // The amount an error type log entry will add to the cascade timer.
const int   Guru::CASCADE_WEIGHT_WARNING =  1;  // The amount a warning type log entry will add to the cascade timer.
const std::string   Guru::FILENAME_LOG =    "log.txt";  // The default name of the log file. Another filename can be specified with open_syslog().


// This has to be a non-class function because C.
void guru_intercept_signal(int sig) { core()->guru()->intercept_signal(sig); }

// Opens the output log for messages.
Guru::Guru(std::string log_filename) : m_cascade_count(0), m_cascade_failure(false), m_cascade_timer(std::time(0)), m_console_ready(false), m_dead_already(false)
{
    if (!log_filename.size()) log_filename = Guru::FILENAME_LOG;
    FileX::delete_file(log_filename);
    m_syslog.open(log_filename.c_str());
    this->log("Guru error-handling system is online. Hooking signals...");
    if (signal(SIGABRT, guru_intercept_signal) == SIG_ERR) halt("Failed to hook abort signal.");
    if (signal(SIGSEGV, guru_intercept_signal) == SIG_ERR) halt("Failed to hook segfault signal.");
    if (signal(SIGILL, guru_intercept_signal) == SIG_ERR) halt("Failed to hook illegal instruction signal.");
    if (signal(SIGFPE, guru_intercept_signal) == SIG_ERR) halt("Failed to hook floating-point exception signal.");
}

// Closes the Guru log file.
Guru::~Guru()
{
    this->log("Guru Meditation system shutting down.");
    this->log("The rest is silence.");
    m_syslog.close();
}

// Tells Guru that we're ready to render Guru error messages on-screen.
void Guru::console_ready(bool is_ready) { m_console_ready = is_ready; }

bool Guru::is_dead() const { return m_dead_already; } // Checks if the system has halted.

// Guru meditation error.
void Guru::halt(const std::string &error)
{
    this->log("Software Failure, Halting Execution", Guru::CRITICAL);
    this->log(error, Guru::CRITICAL);
    if (!m_console_ready) exit(EXIT_FAILURE);

    if (m_dead_already)
    {
        log("Detected cleanup in process, attempting to die peacefully.", Guru::WARN);
        exit(EXIT_FAILURE);
    }
    else m_dead_already = true;

    core()->message("{r}Critical Error: " + error);
    core()->message("{r}Halting execution.");
    do
    {
        core()->messagelog()->render_message_log();
        core()->terminal()->refresh();
    } while (m_console_ready && !core()->terminal()->wants_to_close());
    exit(EXIT_FAILURE);
}

// As above, but with an exception instead of a string.
void Guru::halt(std::exception &e) { halt(e.what()); }

// Catches a segfault or other fatal signal.
void Guru::intercept_signal(int sig)
{
    std::string sig_type;
    switch(sig)
    {
        case SIGABRT: sig_type = "Software requested abort."; break;
        case SIGFPE: sig_type = "Floating-point exception."; break;
        case SIGILL: sig_type = "Illegal instruction."; break;
        case SIGSEGV: sig_type = "Segmentation fault."; break;
        default: sig_type = "Intercepted unknown signal."; break;
    }

    // Disable the signals for now, to stop a cascade.
    signal(SIGABRT, SIG_IGN);
    signal(SIGSEGV, SIG_IGN);
    signal(SIGILL, SIG_IGN);
    signal(SIGFPE, SIG_IGN);
    halt(sig_type);
}

// Logs a message in the system log file.
void Guru::log(std::string msg, int type)
{
    if (!m_syslog.is_open()) return;
    if (msg == m_last_log_message) return;

    m_last_log_message = msg;
    std::string txt_tag;
    switch(type)
    {
        case Guru::INFO: break;
        case Guru::WARN: txt_tag = "[WARN] "; break;
        case Guru::ERROR: txt_tag = "[ERROR] "; break;
        case Guru::CRITICAL: txt_tag = "[CRITICAL] "; break;
    }

    char* buffer = new char[32];
    const time_t now = time(nullptr);
    const tm *ptm = localtime(&now);
    std::strftime(&buffer[0], 32, "%H:%M:%S", ptm);
    std::string time_str = &buffer[0];
    msg = "[" + time_str + "] " + txt_tag + msg;
    m_syslog << msg << std::endl;
    delete[] buffer;
}

// Reports a non-fatal error, which will be logged but will not halt execution unless it cascades.
void Guru::nonfatal(std::string error, int type)
{
    if (m_cascade_failure || m_dead_already) return;
    unsigned int cascade_weight = 0;
    switch(type)
    {
        case Guru::WARN: cascade_weight = Guru::CASCADE_WEIGHT_WARNING; break;
        case Guru::ERROR: cascade_weight = Guru::CASCADE_WEIGHT_ERROR; break;
        case Guru::CRITICAL: cascade_weight = Guru::CASCADE_WEIGHT_CRITICAL; break;
        default: nonfatal("Nonfatal error reported with incorrect severity specified.", Guru::WARN); break;
    }

    this->log(error, type);

    if (cascade_weight)
    {
        unsigned int elapsed_seconds = std::time(0) - m_cascade_timer;
        if (elapsed_seconds <= Guru::CASCADE_TIMEOUT)
        {
            m_cascade_count += cascade_weight;
            if (m_cascade_count > Guru::CASCADE_THRESHOLD)
            {
                m_cascade_failure = true;
                halt("Cascade failure detected!");
            }
        }
        else
        {
            m_cascade_timer = std::time(0);
            m_cascade_count = 0;
        }
    }

    switch(type)
    {
        case Guru::INFO: error = "{b}Info: " + error; break;
        case Guru::WARN: error = "{y}Warning: " + error; break;
        case Guru::ERROR: error = "{r}Error: " + error; break;
        case Guru::CRITICAL: error = "{r}Critical Error: " + error; break;
    }
    core()->message(error);
}
