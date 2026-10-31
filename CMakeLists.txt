cmake_minimum_required(VERSION 3.16)

project(TerminalGames LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(terminal_games
    src/main.cpp
    src/terminal.cpp
    src/racer.cpp
    src/snake.cpp
    src/snake_logic.cpp
)

include(CTest)

if(BUILD_TESTING)
    add_executable(snake_logic_tests
        tests/snake_logic_tests.cpp
        src/snake_logic.cpp
    )
    target_include_directories(snake_logic_tests PRIVATE src)
    add_test(NAME snake_logic_tests COMMAND snake_logic_tests)
endif()

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(terminal_games PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()
