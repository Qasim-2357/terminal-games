#include <iostream>
#include <string>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

bool getTerminalSize(int& width, int& height)
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

void clearTerminal()
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

    FillConsoleOutputCharacter(
        console,
        ' ',
        cellCount,
        home,
        &written
    );

    FillConsoleOutputAttribute(
        console,
        info.wAttributes,
        cellCount,
        home,
        &written
    );

    SetConsoleCursorPosition(console, home);
#else
    std::cout << "\x1B[2J\x1B[H";
    std::cout.flush();
#endif
}

void showTerminalRacer()
{
    int width = 0;
    int height = 0;

    if (!getTerminalSize(width, height))
    {
        std::cout << "Could not determine the terminal dimensions.\n"
                  << "Press M to return to the main menu: ";

        char choice = '\0';
        while (choice != 'm' && choice != 'M')
        {
            std::cin >> choice;
        }
        clearTerminal();
        return;
    }

    const int gameHeight = height - 3;
    if (gameHeight < 5 || width < 7)
    {
        std::cout << "The terminal is too small to display Terminal Racer.\n"
                  << "Press M to return to the main menu: ";

        char choice = '\0';
        while (choice != 'm' && choice != 'M')
        {
            std::cin >> choice;
        }
        clearTerminal();
        return;
    }

    const int roadTop = gameHeight / 3;
    const int roadBottom = gameHeight - 1;
    const int carWidth = 7;
    const int carLeft = (width - carWidth) / 2;
    const int minimumCarTop = roadTop + 1;
    const int maximumCarTop = roadBottom - 2;
    int carTop = maximumCarTop;

#ifndef _WIN32
    termios originalTerminal;
    if (tcgetattr(STDIN_FILENO, &originalTerminal) != 0)
    {
        return;
    }

    termios racerTerminal = originalTerminal;
    racerTerminal.c_lflag &= ~(ICANON | ECHO);
    racerTerminal.c_cc[VMIN] = 1;
    racerTerminal.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &racerTerminal) != 0)
    {
        return;
    }
#endif

    clearTerminal();

    bool playing = true;
    bool firstFrame = true;
    while (playing)
    {
        if (!firstFrame)
        {
#ifdef _WIN32
            HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
            COORD home = {0, 0};
            SetConsoleCursorPosition(console, home);
#else
            std::cout << "\x1B[H";
#endif
        }
        firstFrame = false;

        std::string title(width, ' ');
        const std::string titleText = "TERMINAL RACER";
        for (int column = 0; column < width && column < static_cast<int>(titleText.length()); ++column)
        {
            title[column] = titleText[column];
        }

#ifdef _WIN32
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD titlePosition = {0, 0};
        SetConsoleCursorPosition(console, titlePosition);
#else
        std::cout << "\x1B[1;1H";
#endif
        std::cout << title;

        for (int row = 0; row < gameHeight; ++row)
        {
            std::string line(width, ' ');

            if (row == roadTop || row == roadBottom)
            {
                line.assign(width, '=');
            }
            else if (row > roadTop && row < roadBottom && row == (roadTop + roadBottom) / 2)
            {
                for (int column = 1; column < width; column += 4)
                {
                    line[column] = '-';
                }
            }

            if (row == carTop && carLeft >= 0 && carLeft + carWidth <= width)
            {
                line.replace(carLeft, carWidth, " /---\\ ");
            }
            else if (row == carTop + 1 && carLeft >= 0 && carLeft + carWidth <= width)
            {
                line.replace(carLeft, carWidth, "|_O_O_|");
            }

#ifdef _WIN32
            COORD rowPosition = {0, static_cast<SHORT>(row + 2)};
            SetConsoleCursorPosition(console, rowPosition);
#else
            std::cout << "\x1B[" << row + 3 << ";1H";
#endif
            std::cout << line;
        }

#ifdef _WIN32
        COORD controlsPosition = {0, static_cast<SHORT>(gameHeight + 2)};
        SetConsoleCursorPosition(console, controlsPosition);
#else
        std::cout << "\x1B[" << gameHeight + 3 << ";1H";
#endif
        std::string controls(width, ' ');
        const std::string controlsText = "W: Up  S: Down  M: Main Menu";
        for (int column = 0; column < width && column < static_cast<int>(controlsText.length()); ++column)
        {
            controls[column] = controlsText[column];
        }
        std::cout << controls;

#ifdef _WIN32
        COORD cursorPosition = {0, static_cast<SHORT>(gameHeight + 3)};
        SetConsoleCursorPosition(console, cursorPosition);
#else
        std::cout << "\x1B[" << gameHeight + 4 << ";1H";
#endif
        std::cout.flush();

#ifdef _WIN32
        const char input = static_cast<char>(_getch());
#else
        char input = '\0';
        if (read(STDIN_FILENO, &input, 1) != 1)
        {
            break;
        }
#endif

        if ((input == 'w' || input == 'W') && carTop > minimumCarTop)
        {
            --carTop;
        }
        else if ((input == 's' || input == 'S') && carTop < maximumCarTop)
        {
            ++carTop;
        }
        else if (input == 'm' || input == 'M')
        {
            playing = false;
        }
    }

#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &originalTerminal);
#endif
}

int main()
{
    char choice = '\0';

    while (choice != 'q' && choice != 'Q')
    {
        clearTerminal();
        std::cout << "\nTERMINAL GAMES\n"
                  << "1. Terminal Racer\n"
                  << "2. Coming Soon...\n"
                  << "Q. Quit\n"
                  << "\nChoose an option: ";

        std::cin >> choice;

        if (choice == '1')
        {
            showTerminalRacer();
            choice = '\0';
        }
    }

    return 0;
}
