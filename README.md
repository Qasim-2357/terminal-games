# Terminal Games

Terminal Games is a small C++ terminal-game collection with cross-platform
terminal gameplay.

## Current games

### Terminal Racer

Drive through changing obstacle patterns in a dynamically sized terminal.
Avoid obstacles, move between road rows, and build a persistent best score.

Features:

- ASCII terminal road and vehicle rendering
- Dynamic terminal-size handling
- Pattern-based obstacles with increasing speed and count
- Swept collision detection for fast-moving obstacles
- Score and persistent best score storage
- Ready, pause, game-over, restart, and main-menu screens
- ANSI colors with Windows and POSIX terminal support

### Snake

Guide the snake around the board, eat food, grow longer, and avoid the walls
and your own body. The game ends when the board is full, which is treated as a
win.

Features:

- Bounded ASCII playfield with colored snake and food
- Faster, responsive WASD movement
- Movement speed increases gradually as the score rises
- Up to four regular fruits on the board at once
- A golden `$` special fruit worth five points
- Special fruit expires after five seconds if it is not eaten
- Score and separate persistent best score storage
- Ready, pause, game-over, win, restart, and main-menu screens

## Controls

### Ready screen

- Any key: Start
- `M`: Main menu
- `Q`: Quit

### Gameplay

- `W` / `S`: Move up or down
- `P`: Pause or resume
- `M`: Main menu
- `Q`: Quit

### Game over

- `R`: Play again
- `M`: Main menu
- `Q`: Quit

### Snake

- `W` / `A` / `S` / `D`: Move
- `P`: Pause or resume
- `M`: Main menu
- `Q`: Quit
- `R`: Restart after game over or a full-board win

## Build

Requirements:

- C++17 compiler
- CMake 3.16 or newer

Configure and build with CMake:

```text
cmake -S . -B build
cmake --build build
```

The executable is created at:

```text
build/terminal_games
```

On Windows, use:

```text
build\terminal_games.exe
```

## Run

From the project root:

```text
build/terminal_games
```

On Windows:

```text
build\terminal_games.exe
```

Terminal Racer stores its persistent best score in `best_score.dat`, and Snake
stores its score in `snake_best_score.dat`, beside the executable's working
directory. These local files are ignored by Git.

## Platform support

Terminal Racer uses standard C++ and platform-native terminal APIs for:

- Windows consoles
- Linux terminals
- macOS terminals

No external libraries are required.

## Project structure

```text
.
├── CMakeLists.txt
├── README.md
└── src
    ├── main.cpp
    ├── racer.cpp
    ├── racer.h
    ├── snake.cpp
    ├── snake.h
    ├── snake_logic.cpp
    ├── snake_logic.h
    ├── terminal.cpp
    └── terminal.h
```

## Version

Current version: **v1.0.0**

Additional Terminal Games may be added in future versions.
