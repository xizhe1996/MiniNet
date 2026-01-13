cmake_minimum_required(VERSION 3.15)
project(MiniNet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 统一输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# 编译选项
add_compile_options(-O2 -Wall -Wextra -Wpedantic)

# --- mininet library ---
add_library(mininet STATIC
    src/buffer.cpp
    src/join_thread.cpp
)

target_include_directories(mininet PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# pthread（后面线程池会用到）
find_package(Threads REQUIRED)
target_link_libraries(mininet PUBLIC Threads::Threads)

# --- helper function to add examples ---
function(add_day_example name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE mininet)
endfunction()

# add examples
add_day_example(day01 examples/day01/main.cpp)
add_day_example(day02 examples/day02/main.cpp)
add_day_example(day03 examples/day03/main.cpp)
add_day_example(day04 examples/day04/main.cpp)
add_day_example(day05 examples/day05/main.cpp)
add_day_example(day06 examples/day06/main.cpp)
add_day_example(day08 examples/day08/main.cpp)
add_day_example(day09 examples/day09/main.cpp)
add_day_example(day10 examples/day10/main.cpp)
add_day_example(day11 examples/day11/main.cpp)
add_day_example(day12 examples/day12/main.cpp)
add_day_example(day13 examples/day13/main.cpp)
add_day_example(day14 examples/day14/main.cpp)
add_day_example(day15 examples/day15/main.cpp)
