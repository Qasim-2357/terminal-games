# Terminal Games

Terminal Games is a small C++ terminal-game collection. The current game is
**Terminal Racer**, a cross-platform endless racing obstacle game.

## Current game

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

The game stores the persistent best score in `best_score.dat` beside the
executable's working directory. This local file is ignored by Git.

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
    └── main.cpp
```

## Version

Current version: **v1.0.0**

## Planned

Additional Terminal Games are planned for future versions. They are not
implemented yet.
