#include "tlpp.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <ctime>
#include <iostream>
#include <stdexcept>

namespace lgr {
    void logger::find_logger_args(const int argc, const char **argv) {
        string arg {};
        for (int cur_arg = 1; cur_arg < argc; cur_arg++) {
            arg = string(argv[cur_arg]);
            // search for a valid argument
            if (arg == "-log=verbose")
                this->set_log_level(LOG_LEVEL_VERBOSE);

            else if (arg == "-log=debug")
                this->set_log_level(LOG_LEVEL_DEBUG);

            else if (arg == "-log=info")
                this->set_log_level(LOG_LEVEL_INFO);

            else if (arg == "-log=warn")
                this->set_log_level(LOG_LEVEL_WARN);

            else if (arg == "-log=error")
                this->set_log_level(LOG_LEVEL_ERR);

            else if (arg == "-o") {
                FILE *file = nullptr;
                if (argv[cur_arg + 1])
                    file = fopen(argv[cur_arg + 1], "wa");
                else {
                    errno = EINVAL;
                    throw std::runtime_error("Passed an invalid -o flag");
                }
                if (!file) {
                    errno = ENOENT;
                    throw std::runtime_error("Could not open log output file");
                }
                this->log_file = file;
                this->has_opened_stream = true;
            }

        }
        // didn't find log level args, so set the default one
        if (this->get_log_level() == LOG_LEVEL_UNKN)
            this->set_log_level(DEFAULT_LOG_LEVEL);
    }

    string logger::get_log_color(const LogLevels log_level) const {
        switch (log_level) {
            case LOG_LEVEL_VERBOSE: return c_green;
            case LOG_LEVEL_DEBUG:   return c_purple;
            case LOG_LEVEL_INFO:    return c_blue;
            case LOG_LEVEL_WARN:    return c_yellow;
            case LOG_LEVEL_ERR:     return c_red;
            case LOG_LEVEL_UNKN:
            case MAX_LOG_LEVELS:
            default:                return c_white;
        }
    }

    string logger::get_level_string(const LogLevels level) {
        switch (level) {
            case LOG_LEVEL_VERBOSE: return "VERBOSE";
            case LOG_LEVEL_DEBUG:   return "DEBUG";
            case LOG_LEVEL_INFO:    return "INFO";
            case LOG_LEVEL_WARN:    return "WARNING";
            case LOG_LEVEL_ERR:     return "ERROR";
            case LOG_LEVEL_UNKN:
            case MAX_LOG_LEVELS:
            default:
                                    return "UNKNOWN";
        }
    }

    string logger::get_level_string() {
        return this->get_level_string(this->cur_log_level);
    }

    void logger::set_log_level(const LogLevels level) {
        if (level < 0 || level >= MAX_LOG_LEVELS)
            this->cur_log_level = LOG_LEVEL_UNKN;
        else
            this->cur_log_level = level;
    }

    void logger::set_last_log_time(const time_t time) {
        this->last_log_time = std::max<time_t>(time, 0);
    }

    void logger::increment_written_logs_count() {
        this->written_logs_count++;
    }

    void logger::print_log(
        const std::string_view str,
        const LogLevels log_level,
        const int errno_num,
        const bool include_time,
        const bool include_path,
        const std::chrono::steady_clock::time_point time_point,
        const std::source_location src)
    {
        if (str.empty() && errno_num <= 0) {
            errno = EINVAL;
            return;
        }

        const std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();

        const int err = errno_num;

        if (log_level < this->get_log_level())
            return;

        this->set_last_log_time(time(nullptr));
        const string color = get_log_color(log_level);

        // [LVL] ...
        string log_string = color + "[" + this->get_level_string(log_level) + "] ";

        if (include_path) {
            // [LVL] file:line:column: ...
            log_string.append(string(src.file_name()));
            log_string.append(":");
            log_string.append(std::to_string(src.line()));
            log_string.append(":");
            log_string.append(std::to_string(src.column()) + ": ");
        }

        if (str.empty() && errno_num > 0) {
            // ... Blah-blah error explanation ...
            log_string.append(strerror(err));
        } else if (!str.empty() && errno_num <= 0) {
            // ... Print the passed string ...
            log_string.append(str);
        } else {
            // ... Print the passed string (Blah-blah error explanation) ...
            log_string.append(str);
            const string err_suffix = " (" + string(strerror(err)) + ")";
            log_string.append(err_suffix);
        }

        if (include_time) {
            // ... in 0.002s
            const double exec_s = std::chrono::duration<double>(time_now - time_point).count();
            log_string.append(std::format(" in {:.3f}s", exec_s));
        }

        log_string.append(f_clr);

        std::cout << log_string << '\n';

        if (this->get_stream())
            this->write_log(this->get_log_file(), str, log_level, err, time_point, true);
    }

    void logger::write_log(
        const FILE *file_ptr,
        const std::string_view str,
        const LogLevels log_level,
        const int errno_num,
        const std::chrono::steady_clock::time_point time_point,
        const bool include_time,
        const bool use_ansi,
        const bool include_path,
        const std::source_location src)
    {
        if (!file_ptr) {
            errno = ENOENT;
            this->log_err("Attempted to write a log with an invalid file pointer", true);
        }
        if (str.empty() && errno_num <= 0) {
            errno = EINVAL;
            this->log_err("Could not write a log to a file", true);
        }
        if (log_level < this->get_log_level())
            return;

        const std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
        const int err = errno_num;
        const string color = get_log_color(log_level);
        const tm *tm = localtime(&this->last_log_time);

        this->set_last_log_time(time(nullptr));
        this->increment_written_logs_count();

        // todo: make this a nice little util func, smth like:
        // const string get_fmt_time(string y_m_d_separator = "-", string middle_separator = " ", string h_m_s_separator = ":");
        const string year = std::to_string(tm->tm_year + 1900);
        string month = std::to_string(tm->tm_mon < 10);
        if (month.length() == 1) month.insert(0, "0");
        string day = std::to_string(tm->tm_mday);
        if (day.length() == 1) day.insert(0, "0");
        string hour = std::to_string(tm->tm_hour);
        if (hour.length() == 1) hour.insert(0, "0");
        string min = std::to_string(tm->tm_min);
        if (min.length() == 1) min.insert(0, "0");
        string sec = std::to_string(tm->tm_sec);
        if (sec.length() == 1) sec.insert(0, "0");

        const string time_str = year + "-" + month + "-" + day + " " + hour + ":" + min + ":" + sec + " ";

        // [LVL] ...
        string log_string = time_str + "[" + this->get_level_string(log_level) + "] ";
        if (use_ansi) log_string.insert(0, color);

        if (include_path) {
            // [LVL] file:line:column: ...
            log_string.append(string(src.file_name()));
            log_string.append(":");
            log_string.append(std::to_string(src.line()));
            log_string.append(":");
            log_string.append(std::to_string(src.column()) + ": ");
        }

        if (str.empty() && errno_num > 0) {
            // ... Blah-blah error explanation ...
            log_string.append(strerror(err));
        } else if (!str.empty() && errno_num <= 0) {
            // ... Print the passed string ...
            log_string.append(str);
        } else {
            // ... Print the passed string (Blah-blah error explanation) ...
            log_string.append(str);
            const string err_suffix = " (" + string(strerror(err)) + ")";
            log_string.append(err_suffix);
        }

        if (include_time) {
            // ... in 0.002s
            const double exec_s = std::chrono::duration<double>(time_now - time_point).count();
            log_string.append(std::format(" in {:.3f}s", exec_s));
        }

        if (use_ansi)
            log_string.append(f_clr + '\n');
        else
            log_string.insert(log_string.end(), '\n');
        std::fputs(log_string.c_str(), this->get_log_file());
    }

    void logger::log_verbose(
        const std::string_view str,
        const std::chrono::steady_clock::time_point time_point,
        const bool include_time,
        const bool include_path,
        const int errno_num,
        const LogLevels log_level)
    {
        this->print_log(str, log_level, errno_num, include_time, include_path, time_point);
    }

    void logger::log_debug(
        const std::string_view str,
        const std::chrono::steady_clock::time_point time_point,
        const int errno_num,
        const bool include_time,
        const bool include_path,
        const LogLevels log_level)
    {
        this->print_log(str, log_level, errno_num, include_time, include_path, time_point);
    }

    void logger::log_info(
        const std::string_view str,
        const std::chrono::steady_clock::time_point time_point,
        const bool include_time,
        const bool include_path,
        const int errno_num,
        const LogLevels log_level)
    {
        this->print_log(str, log_level, errno_num, include_time, include_path, time_point);
    }

    void logger::log_warn(
        const std::string_view str,
        const bool include_time,
        const bool include_path,
        const int errno_num,
        const std::chrono::steady_clock::time_point time_point,
        const LogLevels log_level)
    {
        this->print_log(str, log_level, errno_num, include_time, include_path, time_point);
    }

    void logger::log_err(
        const std::string_view str,
        const bool include_path,
        const bool include_time,
        const int errno_num,
        const std::chrono::steady_clock::time_point time_point,
        const LogLevels log_level)
    {
        this->print_log(str, log_level, errno_num, include_time, include_path, time_point);
    }

    logger::logger(const int argc, const char **argv, const LogLevels level) {
        const auto start = std::chrono::steady_clock::now();
        try {
            this->find_logger_args(argc, argv);
        } catch (std::runtime_error &e) {
            this->log_err(e.what());
        }

        if (level != DEFAULT_LOG_LEVEL)
            this->set_log_level(level);

        this->log_verbose("Finished " + this->get_level_string() + " level logger initialization",
            start, true, false);
    }

    logger::~logger() {
        if (!this->get_stream())
            return;

        const time_t last_log = this->get_last_log_time();
        const tm *tm = localtime(&last_log);
        const time_t existence_time = last_log - this->get_creation_time();

        string str = "Requested closure of the " + this->get_level_string() + " level log stream. ";

        // todo: make this a nice little util func, smth like:
        // const string get_fmt_time(string y_m_d_separator = "-", string middle_separator = " ", string h_m_s_separator = ":");
        const string year = std::to_string(tm->tm_year + 1900);
        string month = std::to_string(tm->tm_mon < 10);
        if (month.length() == 1) month.insert(0, "0");
        string day = std::to_string(tm->tm_mday);
        if (day.length() == 1) day.insert(0, "0");
        string hour = std::to_string(tm->tm_hour);
        if (hour.length() == 1) hour.insert(0, "0");
        string min = std::to_string(tm->tm_min);
        if (min.length() == 1) min.insert(0, "0");
        string sec = std::to_string(tm->tm_sec);
        if (sec.length() == 1) sec.insert(0, "0");

        const string time_str = year + "-" + month + "-" + day + " " + hour + ":" + min + ":" + sec + " ";

        str.append("Stream creation time: " + time_str);
        str.append("(existed for " + std::to_string(existence_time) + " seconds).");
        std::fputs(str.c_str(), this->get_log_file());

        if (fclose(this->get_log_file()) != 0) {
            errno = EIO;
            this->log_err("Could not close log file on logger deconstruct");
        }
        this->log_file = nullptr;
        this->has_opened_stream = false;
    }
}
