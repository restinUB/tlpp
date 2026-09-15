#include "tlpp.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <ctime>
#include <iostream>
#include <map>
#include <stdexcept>

namespace lgr {

    const std::map<string, LogLevels> log_levels_args {
        {"-log=verbose", LOG_LEVEL_VERBOSE},
        {"-log=debug", LOG_LEVEL_DEBUG},
        {"-log=info", LOG_LEVEL_INFO},
        {"-log=warn", LOG_LEVEL_WARN},
        {"-log=error", LOG_LEVEL_ERR},
    };

    const string DEFAULT_EXEC_TIME_PREFIX = " in ";

    // ANSI colors
    const string c_black = "\033[0;30m";
    const string c_red = "\033[0;31m";
    const string c_green = "\033[0;32m";
    const string c_yellow = "\033[0;33m";
    const string c_blue = "\033[0;34m";
    const string c_purple = "\033[0;35m";
    const string c_cyan = "\033[0;36m";
    const string c_white = "\033[0;37m";
    // ANSI formatting
    const string f_clr = "\033[0m";
    const string f_bold = "\033[1m";
    const string f_underline = "\033[4m";
    // ANSI backgrounds
    const string bg_black = "\033[40m";
    const string bg_red = "\033[41m";
    const string bg_green = "\033[42m";
    const string bg_yellow = "\033[43m";
    const string bg_blue = "\033[44m";
    const string bg_purple = "\033[45m";
    const string bg_cyan = "\033[46m";
    const string bg_white = "\033[47m";

    FILE *logger::open_log_stream(int argc_num, char *const *argv) {
        FILE *file = nullptr;
        const char *output_file_name = argv[++argc_num];
        if (!output_file_name) {
            errno = EINVAL;
            throw std::runtime_error("Passed an invalid filename after -o flag");
        }

        file = fopen(output_file_name, "wa");
        if (!file) {
            errno = ENOMEM;
            throw std::runtime_error("Could not create log output file");
        }
        return file;
    }

    // I TRIED to keep this readable and short
    void logger::find_logger_args(const int argc, char *const *argv) {
        string arg {};
        bool found_log_level_arg = false;

        for (int cur_arg = 1; cur_arg < argc; cur_arg++) {
            arg = string(argv[cur_arg]);

            // don't waste cycles
            if (found_log_level_arg && arg != "-o")
                break;

            if (arg == "-o") {
                try {
                    this->log_file = open_log_stream(cur_arg, argv);
                    this->has_opened_stream = true;
                } catch (std::runtime_error &e) {
                    this->log_err(e.what());
                }
                break;
            }

            for (const auto &[str, lvl]: log_levels_args) {
                if (arg == str) {
                    this->set_log_level(lvl);
                    found_log_level_arg = true;
                    break;
                }
            }
        }
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
            default:                return "UNKNOWN";
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

    void logger::update_last_log_time(const std::optional<std::chrono::steady_clock::time_point> tp) {
        tp.has_value()
            ? this->last_log_time = tp.value()
            : this->last_log_time = std::chrono::steady_clock::now();
    }

    string logger::get_fmt_time(
        const string &y_m_d_separator,
        const string &middle_separator,
        const string &h_m_s_separator)
    {
        const time_t now = time(nullptr);
        const tm *tm = localtime(&now);

        const string year = std::to_string(tm->tm_year + 1900);

        // we'll add "0" when our num is less than 10
        // e.g. 2026-09-04 instead of 2026-9-4

        string month = std::to_string(tm->tm_mon + 1);
        if (month.length() == 1) month.insert(0, "0");

        string day = std::to_string(tm->tm_mday);
        if (day.length() == 1) day.insert(0, "0");

        string hour = std::to_string(tm->tm_hour);
        if (hour.length() == 1) hour.insert(0, "0");

        string min = std::to_string(tm->tm_min);
        if (min.length() == 1) min.insert(0, "0");

        string sec = std::to_string(tm->tm_sec);
        if (sec.length() == 1) sec.insert(0, "0");

        const string time_str =
            year + y_m_d_separator +
            month + y_m_d_separator +
            day + middle_separator +
            hour + h_m_s_separator +
            min + h_m_s_separator +sec;

        return time_str;
    }

    void logger::print_log(
        const std::string_view str,
        const LogLevels log_level,
        const int errno_num,
        const std::optional<std::chrono::steady_clock::time_point> tp,
        const bool include_path,
        const bool timestamp,
        const bool use_ansi,
        const std::source_location src)
    {
        if (str.empty() && errno_num <= 0) {
            errno = EINVAL;
            return;
        }

        const std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
        this->update_last_log_time(time_now);

        const int err = errno_num;

        if (log_level < this->get_log_level())
            return;

        const string color = get_log_color(log_level);

        string log_string {};
        if (use_ansi)
            log_string.insert(0, color);

        if (timestamp) {
            // "2026-01-14 11:02:29 "
            log_string.append(this->get_fmt_time() + " ");
        }

        // "[LVL] "
        log_string.append( "[" + this->get_level_string(log_level) + "] ");

        if (include_path) {
            // "file:line:column: "
            log_string.append(string(src.file_name()));
            log_string.append(":");
            log_string.append(std::to_string(src.line()));
            log_string.append(":");
            log_string.append(std::to_string(src.column()) + ": ");
        }

        if (str.empty() && errno_num > 0) {
            // "Blah-blah readable errno"
            log_string.append(strerror(err));
        } else if (!str.empty() && errno_num <= 0) {
            // "Passed string"
            log_string.append(str);
        } else {
            // "Passed string (Blah-blah readable errno)"
            log_string.append(str);
            const string err_suffix = " (" + string(strerror(err)) + ")";
            log_string.append(err_suffix);
        }

        double exec_s = -1.0;
        if (tp.has_value()) {
            exec_s = std::chrono::duration<double>(time_now - tp.value()).count();
            // ... in 0.002s
            log_string.append(std::format("{}{:.3f}s", DEFAULT_EXEC_TIME_PREFIX, exec_s));
        }

        if (use_ansi)
            log_string.append(f_clr + '\n');
        else
            log_string.insert(log_string.end(), '\n');

        std::cout << log_string;

        if (this->get_stream())
            this->write_log(str, this->get_log_file(), log_level, exec_s,
                errno_num, true, true, use_ansi, src);
    }

    void logger::write_log(
        const std::string_view str,
        FILE *file_ptr,
        const LogLevels log_level,
        const double exec_time,
        const int errno_num,
        const bool include_path,
        const bool timestamp,
        const bool use_ansi,
        const std::source_location src)
    {
        FILE *write_to = file_ptr
            ? file_ptr
            : this->get_log_file();
        if (!write_to) {
            errno = ENOENT;
            this->log_err("Attempted to write a log with an invalid file pointer");
        }
        if (str.empty() && errno_num <= 0) {
            errno = EINVAL;
            this->log_err("Could not write a log to a file");
        }
        if (log_level < this->get_log_level())
            return;

        const std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
        const int err = errno_num;
        const string color = get_log_color(log_level);

        this->update_last_log_time(time_now);
        this->written_logs_count++;

        string log_string{};

        if (use_ansi)
            log_string.insert(0, color);

        if (timestamp) {
            // "2026-01-14 11:02:29 "
            log_string.append(this->get_fmt_time() + " ");
        }

        // "[LVL] "
        log_string.append( "[" + this->get_level_string(log_level) + "] ");

        if (include_path) {
            // "file:line:column: "
            log_string.append(string(src.file_name()));
            log_string.append(":");
            log_string.append(std::to_string(src.line()));
            log_string.append(":");
            log_string.append(std::to_string(src.column()) + ": ");
        }

        if (str.empty() && errno_num > 0) {
            // "Blah-blah readable errno"
            log_string.append(strerror(err));
        } else if (!str.empty() && errno_num <= 0) {
            // "Passed string"
            log_string.append(str);
        } else {
            // "Passed string (Blah-blah readable errno)"
            log_string.append(str);
            const string err_suffix = " (" + string(strerror(err)) + ")";
            log_string.append(err_suffix);
        }

        if (exec_time >= 0.0) {
            // " in 0.002s"
            log_string.append(std::format("{}{:.3f}s", DEFAULT_EXEC_TIME_PREFIX, exec_time));
        }

        if (use_ansi)
            log_string.append(f_clr + '\n');
        else
            log_string.insert(log_string.end(), '\n');

        std::fputs(log_string.c_str(), write_to);
    }

    void logger::log_verbose(
        const std::string_view str,
        const std::optional<std::chrono::steady_clock::time_point> tp,
        const bool include_path,
        const bool timestamp,
        const int errno_num,
        const LogLevels log_level,
        const bool use_ansi,
        const std::source_location src)
    {
        this->print_log(str, log_level, errno_num, tp, include_path, timestamp, use_ansi, src);
    }

    void logger::log_debug(
        const std::string_view str,
        const std::optional<std::chrono::steady_clock::time_point> tp,
        const bool include_path,
        const bool timestamp,
        const int errno_num,
        const LogLevels log_level,
        const bool use_ansi,
        const std::source_location src)
    {
        this->print_log(str, log_level, errno_num, tp, include_path, timestamp, use_ansi, src);
    }

    void logger::log_info(
        const std::string_view str,
        const std::optional<std::chrono::steady_clock::time_point> tp,
        const bool include_path,
        const bool timestamp,
        const int errno_num,
        const LogLevels log_level,
        const bool use_ansi,
        const std::source_location src)
    {
        this->print_log(str, log_level, errno_num, tp, include_path, timestamp, use_ansi, src);
    }

    void logger::log_warn(
        const std::string_view str,
        const std::optional<std::chrono::steady_clock::time_point> tp,
        const bool include_path,
        const bool timestamp,
        const int errno_num,
        const LogLevels log_level,
        const bool use_ansi,
        const std::source_location src)
    {
        this->print_log(str, log_level, errno_num, tp, include_path, timestamp, use_ansi, src);
    }

    void logger::log_err(
        const std::string_view str,
        const std::optional<std::chrono::steady_clock::time_point> tp,
        const int errno_num,
        const bool include_path,
        const bool timestamp,
        const LogLevels log_level,
        const bool use_ansi,
        const std::source_location src)
    {
        this->print_log(str, log_level, errno_num, tp, include_path, timestamp, use_ansi, src);
    }

    logger::logger(
       const std::optional<int> argc,
       const std::optional<char **> argv,
       const LogLevels level,
       const FILE *log_file)
    {
        const auto start = std::chrono::steady_clock::now();

        this->set_log_level(level);

        if (argc.has_value() && argv.has_value()) {
            try {
                this->find_logger_args(argc.value(), argv.value());
            } catch (std::runtime_error &e) {
                this->log_err(e.what());
            }
        }

        this->log_verbose("Finished " + this->get_level_string() + " level logger initialization", start);
    }

    logger::~logger() {
        if (!this->get_stream())
            return;

        const double existence_time = std::chrono::duration<double>(
            this->get_last_log_time() - this->get_creation_time()).count();

        string str = "Closing " + this->get_level_string() + " level log stream ";
        str.append(std::format("(existed for {:.3f} seconds).", existence_time));

        std::fputs(str.c_str(), this->get_log_file());

        if (fclose(this->get_log_file()) != 0) {
            errno = EIO;
            this->log_err("Could not close log file on logger destruct");
        }
        this->log_file = nullptr;
        this->has_opened_stream = false;
    }
}
