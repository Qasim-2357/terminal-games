#include "terminal.h"

#include <iostream>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace terminal
{
bool getSize(int& width, int& height)
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
    {
        return false;
    }

    width = info.srWindow.Right - info.srWindow.Left + 1;
    height = info.srWindow.Bottom - info.srWindow.Top + 1;
#else
    winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) != 0)
    {
        return false;
    }

    width = size.ws_col;
    height = size.ws_row;
#endif

    return width > 0 && height > 0;
}

void printColored(const std::string& text, const char* color)
{
    std::cout << color << text << "\x1B[0m";
}

void clear()
{
#ifdef _WIN32
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(console, &info))
    {
        return;
    }

    DWORD cellCount = info.dwSize.X * info.dwSize.Y;
    DWORD written = 0;
    COORD home = {0, 0};

    FillConsoleOutputCharacter(console, ' ', cellCount, home, &written);
    FillConsoleOutputAttribute(console, info.wAttributes, cellCount, home, &written);
    SetConsoleCursorPosition(console, home);
#else
    std::cout << "\x1B[2J\x1B[H";
    std::cout.flush();
#endif
}

void moveCursor(int row, int column)
{
#ifdef _WIN32
    COORD position = {
        static_cast<SHORT>(column),
        static_cast<SHORT>(row)};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), position);
#else
    std::cout << "\x1B[" << row + 1 << ";" << column + 1 << "H";
#endif
}

void sleepMilliseconds(int milliseconds)
{
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(static_cast<useconds_t>(milliseconds) * 1000);
#endif
}

bool Session::start()
{
#ifdef _WIN32
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleCursorInfo(console, &originalCursorInfo))
    {
        cursorInfoSaved = true;
        CONSOLE_CURSOR_INFO hiddenCursorInfo = originalCursorInfo;
        hiddenCursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(console, &hiddenCursorInfo);
    }
#else
    if (tcgetattr(STDIN_FILENO, &originalTerminal) != 0)
    {
        return false;
    }

    termios racerTerminal = originalTerminal;
    racerTerminal.c_lflag &= ~(ICANON | ECHO);
    racerTerminal.c_cc[VMIN] = 1;
    racerTerminal.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &racerTerminal) != 0)
    {
        return false;
    }

    std::cout << "\x1B[?25l";
    std::cout.flush();
#endif

    active = true;
    return true;
}

void Session::flushInput()
{
#ifdef _WIN32
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
#else
    char discardedInput[64];
    fd_set pendingInput;
    timeval timeout = {0, 0};

    FD_ZERO(&pendingInput);
    FD_SET(STDIN_FILENO, &pendingInput);
    while (select(STDIN_FILENO + 1, &pendingInput, nullptr, nullptr, &timeout) > 0)
    {
        if (read(STDIN_FILENO, discardedInput, sizeof(discardedInput)) <= 0)
        {
            break;
        }
        FD_ZERO(&pendingInput);
        FD_SET(STDIN_FILENO, &pendingInput);
        timeout = {0, 0};
    }
#endif
}

char Session::pollInput(bool started, bool gameOver, bool& pauseKeyWasDown)
{
    char input = '\0';
#ifdef _WIN32
    if (!started)
    {
        if (GetAsyncKeyState('M') & 0x8000)
        {
            input = 'm';
        }
        else if (GetAsyncKeyState('Q') & 0x8000)
        {
            input = 'q';
        }
        else if (_kbhit())
        {
            const int key = _getch();
            input = static_cast<char>(key == 0 || key == 224 ? 'x' : key);
        }
    }
    else if (GetAsyncKeyState('M') & 0x8000)
    {
        input = 'm';
    }
    else if (GetAsyncKeyState('Q') & 0x8000)
    {
        input = 'q';
    }
    else if (GetAsyncKeyState('W') & 0x8000)
    {
        input = 'w';
    }
    else if (GetAsyncKeyState('S') & 0x8000)
    {
        input = 's';
    }
    else if (GetAsyncKeyState(VK_SPACE) & 0x8000)
    {
        input = ' ';
    }
    else if (gameOver && (GetAsyncKeyState('R') & 0x8000))
    {
        input = 'r';
    }

    const bool pauseKeyIsDown = (GetAsyncKeyState('P') & 0x8000) != 0;
    if (pauseKeyIsDown && !pauseKeyWasDown)
    {
        input = 'p';
    }
    pauseKeyWasDown = pauseKeyIsDown;
#else
    fd_set inputSet;
    FD_ZERO(&inputSet);
    FD_SET(STDIN_FILENO, &inputSet);
    timeval timeout = {0, 0};

    if (select(STDIN_FILENO + 1, &inputSet, nullptr, nullptr, &timeout) > 0)
    {
        if (read(STDIN_FILENO, &input, 1) != 1)
        {
            return '\0';
        }
    }
#endif
    return input;
}

void Session::stop()
{
    if (!active)
    {
        return;
    }

    flushInput();
#ifdef _WIN32
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (cursorInfoSaved)
    {
        SetConsoleCursorInfo(console, &originalCursorInfo);
    }
#else
    tcsetattr(STDIN_FILENO, TCSANOW, &originalTerminal);
    std::cout << "\x1B[?25h";
    std::cout.flush();
#endif
    active = false;
}
}
