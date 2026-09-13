# Tiny logger ++
This is an educational project I made as a replacement for my exact same C logger.
I've never used other logging libraries, so this lib is my pure rookie look.

## Some small features:
- formatted terminal output using ANSI and a set of log "segments"
- append to a specified file (with optional ANSI support)
- measure elapsed time between time points
- several log levels

```c++
#include "libpp/logging.hpp"
#include <chrono>

// dummy func to create a noticeable delay between initial_time_point and log call
void do_some_work(const int iterations = 0){
    for (int i = 0; i < iterations; i++);
}

int main(const int argc, const char **argv) {
    const auto initial_time_point = std::chrono::steady_clock::now();
    
    lgr::logger logger(argc, argv);
    do_some_work(100'000'000);
    // true means that we want to include_time
    logger.log_info("Displayed a test log", initial_time_point, true); 
    return 0;
}

```
**Output:** `[INFO] Displayed a test log in 0.312s`
```css
[INFO] Displayed a test log in 0.312s
^^^^^^ ^^^^^^^^^^^^^^^^^^^^ ^^^^^^^^^
│      │                    └ calculated time between
│      └──── custom string    initial_time_point and log action
└ log level
```

**Explanation:**
A log consists of several parts: timestamp, log level, file path, a message, and elapsed time.

Example:
```cmake
2026-01-13 04:20:41 [VERBOSE] F:/Files/CPP/untitled/libpp/logging.cpp:163:28: Finished VERBOSE level logger initialization in 0.001s
```