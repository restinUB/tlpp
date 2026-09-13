#pragma once

#include <chrono>
#include <ctime>
#include <functional>
#include <source_location>
#include <string>

namespace lgr {
    typedef std::string string;

    #define DEFAULT_LOG_LEVEL LOG_LEVEL_INFO
    #define USE_ANSI_ON_LOG_WRITE false

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

    class logger {
    public:
        typedef enum LogLevels {
            LOG_LEVEL_VERBOSE,
            LOG_LEVEL_DEBUG,
            LOG_LEVEL_INFO,
            LOG_LEVEL_WARN,
            LOG_LEVEL_ERR,
            LOG_LEVEL_UNKN,
            MAX_LOG_LEVELS, // only for range checks
        } LogLevels;

    private:
        LogLevels cur_log_level = LOG_LEVEL_UNKN;

        time_t last_log_time {};
        time_t creation_time {time(nullptr)};
        unsigned written_logs_count {};

        bool has_opened_stream {};
        FILE *log_file {};

        void find_logger_args(int argc, const char **argv);

        void print_log(
            std::string_view str,
            LogLevels log_level = DEFAULT_LOG_LEVEL,
            int errno_num = errno,
            bool include_time = false,
            bool include_path = false,
            std::chrono::steady_clock::time_point time_point = {},
            std::source_location src = std::source_location::current());

        void write_log(
            const FILE *file_ptr,
            std::string_view str,
            LogLevels log_level = DEFAULT_LOG_LEVEL,
            int errno_num = errno,
            std::chrono::steady_clock::time_point time_point = {},
            bool include_time = false,
            bool use_ansi = USE_ANSI_ON_LOG_WRITE,
            bool include_path = true,
            std::source_location src = std::source_location::current());

    public:
        [[nodiscard]]
        string get_log_color(LogLevels log_level) const;

        string get_level_string(LogLevels level);

        string get_level_string();

        void set_log_level(LogLevels level);

        [[nodiscard]]
        LogLevels get_log_level() const {return this->cur_log_level;};

        void set_last_log_time(time_t time);

        [[nodiscard]]
        time_t get_last_log_time() const {return this->last_log_time;};

        [[nodiscard]]
        time_t get_creation_time() const {return this->creation_time;};

        void increment_written_logs_count();

        [[nodiscard]]
        unsigned get_written_logs_count() const {return this->written_logs_count;};

        [[nodiscard]]
        bool get_stream() const {return this->has_opened_stream;};

        [[nodiscard]]
        FILE *get_log_file() const {return this->log_file;};

        void log_verbose(
            std::string_view str,
            std::chrono::steady_clock::time_point time_point = {},
            bool include_time = false,
            bool include_path = true,
            int errno_num = 0,
            LogLevels log_level = LOG_LEVEL_VERBOSE);

        void log_debug(
            std::string_view str,
            std::chrono::steady_clock::time_point time_point = {},
            int errno_num = 0,
            bool include_time = false,
            bool include_path = false,
            LogLevels log_level = LOG_LEVEL_DEBUG);

        void log_info(
            std::string_view str,
            std::chrono::steady_clock::time_point time_point = {},
            bool include_time = false,
            bool include_path = false,
            int errno_num = 0,
            LogLevels log_level = LOG_LEVEL_INFO);

        void log_warn(
            std::string_view str,
            bool include_time = false,
            bool include_path = false,
            int errno_num = 0,
            std::chrono::steady_clock::time_point time_point = {},
            LogLevels log_level = LOG_LEVEL_WARN);

        void log_err(
            std::string_view str,
            bool include_path = false,
            bool include_time = false,
            int errno_num = errno,
            std::chrono::steady_clock::time_point time_point = {},
            LogLevels log_level = LOG_LEVEL_ERR);

        logger(int argc, const char **argv, LogLevels level = DEFAULT_LOG_LEVEL);

        ~logger();
    };
}
