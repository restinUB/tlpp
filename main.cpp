#include "tlpp.hpp"
#include <chrono>

// some useless func to create a noticeable delay between initial_time_point and log call
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
