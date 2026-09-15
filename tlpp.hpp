#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <source_location>
#include <string>

namespace lgr {
    typedef std::string string;

    typedef enum LogLevels {
        LOG_LEVEL_VERBOSE,
        LOG_LEVEL_DEBUG,
        LOG_LEVEL_INFO,
        LOG_LEVEL_WARN,
        LOG_LEVEL_ERR,
        LOG_LEVEL_UNKN,
        MAX_LOG_LEVELS, // only for range checks
    } LogLevels;

    #define DEFAULT_LOG_LEVEL LOG_LEVEL_INFO

    #define DEFAULT_TIMESTAMP_ON_LOG_PRINT false
    #define USE_ANSI_ON_LOG_PRINT true

    #define DEFAULT_TIMESTAMP_ON_LOG_WRITE true
    #define USE_ANSI_ON_LOG_WRITE false
    
    extern const string DEFAULT_EXEC_TIME_PREFIX;

    extern const std::map<string, LogLevels> log_levels_args;

    // ANSI colors
    extern const string c_black;
    extern const string c_red;
    extern const string c_green;
    extern const string c_yellow;
    extern const string c_blue;
    extern const string c_purple;
    extern const string c_cyan;
    extern const string c_white;
    // ANSI formatting
    extern const string f_clr;
    extern const string f_bold;
    extern const string f_underline;
    // ANSI backgrounds
    extern const string bg_black;
    extern const string bg_red;
    extern const string bg_green;
    extern const string bg_yellow;
    extern const string bg_blue;
    extern const string bg_purple;
    extern const string bg_cyan;
    extern const string bg_white;

    class logger {
    private:
        LogLevels cur_log_level = LOG_LEVEL_UNKN;

        std::chrono::steady_clock::time_point last_log_time {};
        std::chrono::steady_clock::time_point creation_time = std::chrono::steady_clock::now();
        unsigned written_logs_count {};

        bool has_opened_stream {};
        FILE *log_file {};

        void find_logger_args(int argc, char *const *argv);

        FILE *open_log_stream(int argc_num, char *const *argv);

        string get_fmt_time(
            const string &y_m_d_separator = "-",
            const string &middle_separator = " ",
            const string &h_m_s_separator = ":");

        void print_log(
            std::string_view str,
            LogLevels log_level = DEFAULT_LOG_LEVEL,
            int errno_num = errno,
            std::optional<std::chrono::steady_clock::time_point> tp = std::nullopt,
            bool include_path = false,
            bool timestamp = DEFAULT_TIMESTAMP_ON_LOG_PRINT,
            bool use_ansi = USE_ANSI_ON_LOG_PRINT,
            std::source_location src = std::source_location::current());

        void write_log(
            std::string_view str,
            FILE *file_ptr = nullptr,
            LogLevels log_level = DEFAULT_LOG_LEVEL,
            double exec_time = -1.0,
            int errno_num = errno,
            bool include_path = true,
            bool timestamp = DEFAULT_TIMESTAMP_ON_LOG_WRITE,
            bool use_ansi = USE_ANSI_ON_LOG_WRITE,
            std::source_location src = std::source_location::current());

    public:
        [[nodiscard]]
        string get_log_color(LogLevels log_level) const;

        string get_level_string(LogLevels level);

        string get_level_string();

        void set_log_level(LogLevels level);

        [[nodiscard]]
        LogLevels get_log_level() const {return this->cur_log_level;};

        void update_last_log_time(
            std::optional<std::chrono::steady_clock::time_point> tp = std::nullopt);

        [[nodiscard]]
        std::chrono::steady_clock::time_point get_last_log_time() const {return this->last_log_time;};

        [[nodiscard]]
        std::chrono::steady_clock::time_point get_creation_time() const {return this->creation_time;};

        [[nodiscard]]
        unsigned get_written_logs_count() const {return this->written_logs_count;};

        [[nodiscard]]
        bool get_stream() const {return this->has_opened_stream;};

        [[nodiscard]]
        FILE *get_log_file() const {return this->log_file;};

        void log_verbose(
            std::string_view str,
            std::optional<std::chrono::steady_clock::time_point> tp = std::nullopt,
            bool include_path = true,
            bool timestamp = DEFAULT_TIMESTAMP_ON_LOG_PRINT,
            int errno_num = 0,
            LogLevels log_level = LOG_LEVEL_VERBOSE,
            bool use_ansi = USE_ANSI_ON_LOG_PRINT,
            std::source_location src = std::source_location::current());

        void log_debug(
            std::string_view str,
            std::optional<std::chrono::steady_clock::time_point> tp = std::nullopt,
            bool include_path = false,
            bool timestamp = DEFAULT_TIMESTAMP_ON_LOG_PRINT,
            int errno_num = 0,
            LogLevels log_level = LOG_LEVEL_DEBUG,
            bool use_ansi = USE_ANSI_ON_LOG_PRINT,
            std::source_location src = std::source_location::current());

        void log_info(
            std::string_view str,
            std::optional<std::chrono::steady_clock::time_point> tp = std::nullopt,
            bool include_path = false,
            bool timestamp = DEFAULT_TIMESTAMP_ON_LOG_PRINT,
            int errno_num = 0,
            LogLevels log_level = LOG_LEVEL_INFO,
            bool use_ansi = USE_ANSI_ON_LOG_PRINT,
            std::source_location src = std::source_location::current());

        void log_warn(
            std::string_view str,
            std::optional<std::chrono::steady_clock::time_point> tp = std::nullopt,
            bool include_path = false,
            bool timestamp = DEFAULT_TIMESTAMP_ON_LOG_PRINT,
            int errno_num = 0,
            LogLevels log_level = LOG_LEVEL_WARN,
            bool use_ansi = USE_ANSI_ON_LOG_PRINT,
            std::source_location src = std::source_location::current());

        void log_err(
            std::string_view str,
            std::optional<std::chrono::steady_clock::time_point> tp = std::nullopt,
            int errno_num = errno,
            bool include_path = true,
            bool timestamp = DEFAULT_TIMESTAMP_ON_LOG_PRINT,
            LogLevels log_level = LOG_LEVEL_ERR,
            bool use_ansi = USE_ANSI_ON_LOG_PRINT,
            std::source_location src = std::source_location::current());

        logger(
            std::optional<int> argc = std::nullopt,
            std::optional<char **> argv = std::nullopt,
            LogLevels level = DEFAULT_LOG_LEVEL,
            const FILE *log_file = nullptr);

        ~logger();
    };
}
