#ifndef TERMINAL_H
#define TERMINAL_H

#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#endif

namespace terminal
{
bool getSize(int& width, int& height);
void clear();
void printColored(const std::string& text, const char* color);
void moveCursor(int row, int column);
void sleepMilliseconds(int milliseconds);

class Session
{
public:
    bool start();
    void flushInput();
    char pollInput(bool started, bool gameOver, bool& pauseKeyWasDown);
    void stop();

private:
    bool active = false;
#ifdef _WIN32
    CONSOLE_CURSOR_INFO originalCursorInfo = {};
    bool cursorInfoSaved = false;
#else
    termios originalTerminal = {};
#endif
};
}

#endif
