# Tiny logger ++
A lightweight (~17KiB) C++ logger lib for small projects.

<img width="515" height="101" alt="image" src="https://github.com/user-attachments/assets/149eb0e2-c6da-42e2-8000-696d9d7f8852" />

<details>

<summary>Some more details and how it's started</summary>

This is an educational project I created as a replacement for my exact same C logger.
I wanted to use some modern C++ features, so I rewrote it using them.
And I prefer using standard C++ strings. Honestly, you probably wouldn't like the mess I made in the original C lib.

Since I haven't used other logging libraries before, this one is my rookie look on the problem. 
It might be memory-inefficient or look a bit rough.
To be honest, I just wanted to practice building something that actually works and is at least 1% useful.

It was also my first time using certain C++ features like `std::chrono`, `std::source_location`, and `std::string_view`.
I don't even fully understand how they work yet. Anyway, it compiles and runs, so I'm happy with that.

</details>

## Some small features
- detailed log entries with timestamp, log level, custom message, file path, line number, column, and execution time
- standard logging levels (verbose, debug, info, warning, error) with configurable filtering
- logs can be directed to the console or appended to a specified file (with ANSI color support)
- easy-to-use support for measuring elapsed time between code execution points using `std::chrono`
- `errno` integration

## Usage guide
**Clone:** `git clone https://github.com/restinUB/tlpp.git`
### Setup
You must initialize the `lgr::logger` object to start. Pass the standard CLI `argc`, `argv` arguments to allow the logger to automatically parse configuration flags.
```c++
#include "tlpp.hpp"

int main(const int argc, const char **argv) {
    // initialize the logger object
    lgr::logger logger(argc, argv);
    // the logger automatically parses flags like -log=info or -o <path>
    // and you're ready to log!
    return 0;
}
```
### Logging
Tlpp provides dedicated methods for each log level. This is intended to make log calls short, with as much context you'll need for one of another log level (e.g., `log_err()` automatically uses `errno`, so there's no need to explicitly specify it).
| method                                | description                            | use case                                               |
| :------------------------------------ | :------------------------------------- | :----------------------------------------------------- |
| `logger.log_info(str, ...) `          | standard info messages                 | general flow tracking, best for execution time logging |
| `logger.log_debug(str, ...)`          | detailed debugging info                | tracing variable states and execution paths            |
| `logger.log_warn(str, ...)`           | warns that should be investigated      | deprecated features or unexpected conditions           |
| `logger.log_err(str, ..., errno_num)` | critical errors requiring attention    | handling system failures (`ENOMEM`, `ENOENT`, ...)     |
| `logger.log_verbose(str, ...)`        | highly detailed logs  for deep tracing | debugging complex internal logic                       |

<details>

<summary><b>A quick example</b></summary>

```c++
// log a simple info message
logger.log_info("User logged in successfully"); // no full stop needed

// log an error
logger.log_err("Failed to allocate memory.", true);
//                                           ^^^^
//                  include_path (default: false)
//
// You can also use include_time (bool) after
// include_path, but make sure to pass time_point.
// Errno will be captured automatically

// measure time and log a complex event
const auto start_time = std::chrono::steady_clock::now();

// simulate work (in this example, an empty for() cycle with 100.000.000 iterations)
do_some_work(100'000'000);

logger.log_info("Processing complete", start_time, true);
//                                                 ^^^^
//                                         include_time
```
**Output:**
```css
[INFO] Processing complete in 0.312s
^^^^^^ ^^^^^^^^^^^^^^^^^^^ ^^^^^^^^^
│      │                   └> 'calculated time between'
│      │                      'start_time and log action'
│      │
│      └────────────────────> 'custom string'
│
└───────────────────────────> 'log level'
```

</details>

<details>

<summary><h3>CLI arguments</h3></summary>

The logger automatically processes the following command-line arguments upon initialization:
| flag           | description                                                                | example             |
| :------------- | :------------------------------------------------------------------------- | :------------------ |
| `-log=<level>` | sets the minimum log level to be displayed/written                         | `my_app -log=warn`  |
| `-o <path>`    | duplicates all logs to a specified file path with a timestamp for each log | `my_app -o app.log` |

### Available log levels
- `-log=verbose`: logs everything (verbose, debug, info, warn, error)
- `-log=debug`: logs debug, info, warn, and error messages
- `-log=info`: logs info, warn, and error messages
- `-log=warn`: logs warning and error messages only
- `-log=error`: logs only error messages

</details>
