cmake_minimum_required(VERSION 4.3)
project(tlpp CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(tlpp STATIC
    tlpp.cpp
)

target_include_directories(tlpp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(tlpp_bin
    main.cpp
)

target_link_libraries(tlpp_bin
    PRIVATE
        tlpp
)
