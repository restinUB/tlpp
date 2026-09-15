#include "tlpp.hpp"
#include <chrono>

// some useless func to create a noticeable delay between initial_time_point and log call
void do_some_work(const int iterations = 0){
    for (int i = 0; i < iterations; i++);
}

int main(const int argc, char **argv) {
    const auto initial_time_point = std::chrono::steady_clock::now();

    lgr::logger logger(argc, argv);
    do_some_work(100'000'000);
    logger.log_info("Displayed a test log", initial_time_point);
    return 0;
}
