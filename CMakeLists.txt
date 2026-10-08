cmake_minimum_required(VERSION 3.16)

project(TerminalGames LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(terminal_games
    src/main.cpp
)

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(terminal_games PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()
