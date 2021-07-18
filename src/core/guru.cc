    // core/guru.cc -- Guru Meditation error-handling and reporting system.
    // Copyright (c) 2020-2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

    #include "core/core.h"
    #include "core/filex.h"
    #include "core/guru.h"

    #include <csignal>
    #include <sstream>


    constexpr char  Guru::FILENAME_LOG[] = "log.txt";   // The default name of the log file. Another filename can be specified with open_syslog().


    // This has to be a non-class function because C.
    void guru_intercept_signal(int sig) { core()->guru()->intercept_signal(sig); }

    // Opens the output log for messages.
    Guru::Guru(std::string log_filename) : cache_nonfatal_(false), cascade_count_(0), cascade_failure_(false), cascade_timer_(std::time(0)), console_ready_(false), dead_already_(false)
    {
        if (!log_filename.size()) log_filename = Guru::FILENAME_LOG;
        FileX::delete_file(log_filename);
        syslog_.open(log_filename.c_str());
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
        syslog_.close();
    }

    // Enables or disables cache of nonfatal error messages.
    void Guru::cache_nonfatal(bool cache)
    {
        cache_nonfatal_ = cache;
        if (!cache) nonfatal_cache_.clear();
    }

    // Tells Guru that we're ready to render Guru error messages on-screen.
    void Guru::console_ready(bool is_ready) { console_ready_ = is_ready; }

    // Dumps all cached nonfatal messages to the console.
    void Guru::dump_nonfatal()
    {
        if (!console_ready_)
        {
            nonfatal("Attempt to dump nonfatal errors before console is initialized!", Guru::GURU_WARN);
            return;
        }
        for (auto message : nonfatal_cache_)
            core()->message(message);
        cache_nonfatal(false);
    }

    bool Guru::is_dead() const { return dead_already_; } // Checks if the system has halted.

    // Guru meditation error.
    void Guru::halt(const std::string &error)
    {
        this->log("Software Failure, Halting Execution", Guru::GURU_CRITICAL);
        this->log(error, Guru::GURU_CRITICAL);
        if (!console_ready_) exit(EXIT_FAILURE);

        if (dead_already_)
        {
            log("Detected cleanup in process, attempting to die peacefully.", Guru::GURU_WARN);
            exit(EXIT_FAILURE);
        }
        else dead_already_ = true;

        core()->message("{r}Critical Error: " + error);
        core()->message("{r}Halting execution.");
        core()->messagelog()->render_message_log();
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
        if (!syslog_.is_open()) return;
        if (msg == last_log_message_) return;

        last_log_message_ = msg;
        std::string txt_tag;
        switch(type)
        {
            case Guru::GURU_INFO: break;
            case Guru::GURU_WARN: txt_tag = "[WARN] "; break;
            case Guru::GURU_ERROR: txt_tag = "[ERROR] "; break;
            case Guru::GURU_CRITICAL: txt_tag = "[CRITICAL] "; break;
        }

        char* buffer = new char[32];
        const time_t now = time(nullptr);
        const tm *ptm = localtime(&now);
        std::strftime(&buffer[0], 32, "%H:%M:%S", ptm);
        std::string time_str = &buffer[0];
        msg = "[" + time_str + "] " + txt_tag + msg;
        syslog_ << msg << std::endl;
        delete[] buffer;
    }

    // Reports a non-fatal error, which will be logged but will not halt execution unless it cascades.
    void Guru::nonfatal(std::string error, int type)
    {
        if (cascade_failure_ || dead_already_) return;
        int cascade_weight = 0;
        switch(type)
        {
            case Guru::GURU_WARN: cascade_weight = Guru::CASCADE_WEIGHT_WARNING; break;
            case Guru::GURU_ERROR: cascade_weight = Guru::CASCADE_WEIGHT_ERROR; break;
            case Guru::GURU_CRITICAL: cascade_weight = Guru::CASCADE_WEIGHT_CRITICAL; break;
            default: nonfatal("Nonfatal error reported with incorrect severity specified.", Guru::GURU_WARN); break;
        }

        this->log(error, type);

        if (cascade_weight)
        {
            time_t elapsed_seconds = std::time(0) - cascade_timer_;
            if (elapsed_seconds <= Guru::CASCADE_TIMEOUT)
            {
                cascade_count_ += cascade_weight;
                if (cascade_count_ > Guru::CASCADE_THRESHOLD)
                {
                    cascade_failure_ = true;
                    halt("Cascade failure detected!");
                }
            }
            else
            {
                cascade_timer_ = std::time(0);
                cascade_count_ = 0;
            }
        }

        switch(type)
        {
            case Guru::GURU_INFO: error = "{U}Info: " + error; break;
            case Guru::GURU_WARN: error = "{Y}Warning: " + error; break;
            case Guru::GURU_ERROR: error = "{R}Error: " + error; break;
            case Guru::GURU_CRITICAL: error = "{M}Critical Error: " + error; break;
        }
        if (core()->messagelog() != nullptr) core()->message(error);
        else throw std::runtime_error(error);
        if (cache_nonfatal_) nonfatal_cache_.push_back(error);
    }
