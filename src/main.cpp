#include <iostream>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/select.h>
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

void showTerminalRacer(int& bestScore)
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

    const int roadTop = 0;
    const int roadBottom = 11;
    const int gameHeight = roadBottom + 1;
    if (height < gameHeight + 5 || width < 7)
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

    const int carWidth = 7;
    const int carLeft = (width - carWidth) / 2;
    const int minimumCarTop = roadTop + 1;
    const int maximumCarTop = roadBottom - 2;
    const int obstacleWidth = 3;
    const int obstaclePatternRows[8] = {
        roadTop + 1,
        roadTop + 2,
        roadBottom - 2,
        roadBottom - 1,
        roadTop + 3,
        roadBottom - 3,
        roadTop + 4,
        roadBottom - 4
    };

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

#ifdef _WIN32
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO originalCursorInfo;
    bool cursorInfoSaved = GetConsoleCursorInfo(console, &originalCursorInfo) != 0;

    if (cursorInfoSaved)
    {
        CONSOLE_CURSOR_INFO hiddenCursorInfo = originalCursorInfo;
        hiddenCursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(console, &hiddenCursorInfo);
    }
#else
    std::cout << "\x1B[?25l";
    std::cout.flush();
#endif

    bool playing = true;
    while (playing)
    {
        int carTop = maximumCarTop;
        const int obstacleSpacing = width / 12 > 5 ? width / 12 : 6;
        int obstacleLeft[8] = {
            width - obstacleWidth,
            width + obstacleSpacing,
            width + obstacleSpacing * 2,
            width + obstacleSpacing * 3,
            width + obstacleSpacing * 4,
            width + obstacleSpacing * 5,
            width + obstacleSpacing * 6,
            width + obstacleSpacing * 7
        };
        int obstacleRow[8] = {
            obstaclePatternRows[0],
            obstaclePatternRows[1],
            obstaclePatternRows[2],
            obstaclePatternRows[3],
            obstaclePatternRows[4],
            obstaclePatternRows[5],
            obstaclePatternRows[6],
            obstaclePatternRows[7]
        };
        int score = 0;
        int activeObstacleCount = 3;
        bool gameOver = false;
        bool firstFrame = true;

        while (playing)
        {
        char input = '\0';
#ifdef _WIN32
        if (GetAsyncKeyState('M') & 0x8000)
        {
            input = 'm';
        }
        else if (GetAsyncKeyState('W') & 0x8000)
        {
            input = 'w';
        }
        else if (GetAsyncKeyState('S') & 0x8000)
        {
            input = 's';
        }
        else if (gameOver && (GetAsyncKeyState('R') & 0x8000))
        {
            input = 'r';
        }
#else
        fd_set inputSet;
        FD_ZERO(&inputSet);
        FD_SET(STDIN_FILENO, &inputSet);
        timeval timeout = {0, 0};

        if (select(STDIN_FILENO + 1, &inputSet, nullptr, nullptr, &timeout) > 0)
        {
            if (read(STDIN_FILENO, &input, 1) != 1)
            {
                break;
            }
        }
#endif

        if (!gameOver && (input == 'w' || input == 'W') && carTop > minimumCarTop)
        {
            --carTop;
        }
        else if (!gameOver && (input == 's' || input == 'S') && carTop < maximumCarTop)
        {
            ++carTop;
        }
        else if (input == 'm' || input == 'M')
        {
            playing = false;
        }
        else if (gameOver && (input == 'r' || input == 'R'))
        {
            break;
        }

        if (!playing)
        {
            break;
        }

        if (!gameOver)
        {
            int obstacleSpeed = 2;
            if (score >= 30)
            {
                obstacleSpeed = 6;
            }
            else if (score >= 20)
            {
                obstacleSpeed = 5;
            }
            else if (score >= 10)
            {
                obstacleSpeed = 4;
            }
            else if (score >= 5)
            {
                obstacleSpeed = 3;
            }

            for (int obstacle = 0; obstacle < activeObstacleCount; ++obstacle)
            {
                const int previousLeft = obstacleLeft[obstacle];
                const int movedLeft = previousLeft - obstacleSpeed;
                const int pathLeft = movedLeft < 0 ? 0 : movedLeft;
                const bool horizontalPathOverlap =
                    carLeft < previousLeft + obstacleWidth &&
                    carLeft + carWidth > pathLeft;
                const bool verticalOverlap =
                    obstacleRow[obstacle] >= carTop &&
                    obstacleRow[obstacle] <= carTop + 1;

                if (horizontalPathOverlap && verticalOverlap)
                {
                    gameOver = true;
                }

                obstacleLeft[obstacle] = movedLeft;
                if (obstacleLeft[obstacle] + obstacleWidth <= 0)
                {
                    int furthestObstacle = width;
                    for (int other = 0; other < activeObstacleCount; ++other)
                    {
                        if (other != obstacle && obstacleLeft[other] > furthestObstacle)
                        {
                            furthestObstacle = obstacleLeft[other];
                        }
                    }

                    obstacleLeft[obstacle] = furthestObstacle + obstacleSpacing;
                    obstacleRow[obstacle] =
                        obstaclePatternRows[(obstacle + score) % 8];
                    ++score;
                    if (score > bestScore)
                    {
                        bestScore = score;
                        std::ofstream bestScoreFile("best_score.dat", std::ios::trunc);
                        if (bestScoreFile)
                        {
                            bestScoreFile << bestScore;
                        }
                    }
                }

                if (gameOver)
                {
                    break;
                }
            }

            const int desiredObstacleCount =
                score >= 20 ? 5 : (score >= 10 ? 4 : 3);
            while (activeObstacleCount < desiredObstacleCount)
            {
                int furthestObstacle = width;
                for (int obstacle = 0; obstacle < activeObstacleCount; ++obstacle)
                {
                    if (obstacleLeft[obstacle] > furthestObstacle)
                    {
                        furthestObstacle = obstacleLeft[obstacle];
                    }
                }

                obstacleLeft[activeObstacleCount] =
                    furthestObstacle + obstacleSpacing;
                obstacleRow[activeObstacleCount] =
                    obstaclePatternRows[activeObstacleCount];
                ++activeObstacleCount;
            }
        }

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
        std::cout.flush();

#ifdef _WIN32
        COORD scorePosition = {0, 1};
        SetConsoleCursorPosition(console, scorePosition);
#else
        std::cout << "\x1B[2;1H";
#endif
        std::string scoreboard(width, ' ');
        const std::string scoreText = "Score: " + std::to_string(score);
        for (int column = 0; column < width && column < static_cast<int>(scoreText.length()); ++column)
        {
            scoreboard[column] = scoreText[column];
        }
        std::cout << scoreboard;
        std::cout.flush();

#ifdef _WIN32
        COORD bestScorePosition = {0, 2};
        SetConsoleCursorPosition(console, bestScorePosition);
#else
        std::cout << "\x1B[3;1H";
#endif
        std::string bestScoreboard(width, ' ');
        const std::string bestScoreText = "Best: " + std::to_string(bestScore);
        for (int column = 0; column < width && column < static_cast<int>(bestScoreText.length()); ++column)
        {
            bestScoreboard[column] = bestScoreText[column];
        }
        std::cout << bestScoreboard;
        std::cout.flush();

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

            for (int obstacle = 0; obstacle < activeObstacleCount; ++obstacle)
            {
                for (int column = 0; column < obstacleWidth; ++column)
                {
                    const int obstacleColumn = obstacleLeft[obstacle] + column;
                    if (obstacleColumn >= 0 && obstacleColumn < width)
                    {
                        if (row == obstacleRow[obstacle])
                        {
                            line[obstacleColumn] = '#';
                        }
                    }
                }
            }

            if (!gameOver && row == carTop && carLeft >= 0 && carLeft + carWidth <= width)
            {
                line.replace(carLeft, carWidth, " /---\\ ");
            }
            else if (!gameOver && row == carTop + 1 && carLeft >= 0 && carLeft + carWidth <= width)
            {
                line.replace(carLeft, carWidth, "|_O_O_|");
            }

            if (gameOver)
            {
                const int popupRow = (roadTop + roadBottom) / 2 - 2;
                std::string gameOverText;

                if (row == popupRow)
                {
                    gameOverText = "GAME OVER";
                }
                else if (row == popupRow + 1)
                {
                    gameOverText = "Score: " + std::to_string(score);
                }
                else if (row == popupRow + 2)
                {
                    gameOverText = "Best: " + std::to_string(bestScore);
                }
                else if (row == popupRow + 3)
                {
                    gameOverText = "R: Play Again";
                }
                else if (row == popupRow + 4)
                {
                    gameOverText = "M: Main Menu";
                }

                if (!gameOverText.empty())
                {
                    const int textLeft = (width - static_cast<int>(gameOverText.length())) / 2;
                    if (textLeft >= 0 && textLeft + static_cast<int>(gameOverText.length()) <= width)
                    {
                        line.replace(textLeft, gameOverText.length(), gameOverText);
                    }
                }
            }
#ifdef _WIN32
            COORD rowPosition = {0, static_cast<SHORT>(row + 3)};
            SetConsoleCursorPosition(console, rowPosition);
#else
            std::cout << "\x1B[" << row + 4 << ";1H";
#endif
            std::cout << line;
            std::cout.flush();
        }

#ifdef _WIN32
        COORD controlsPosition = {0, static_cast<SHORT>(gameHeight + 3)};
        SetConsoleCursorPosition(console, controlsPosition);
#else
        std::cout << "\x1B[" << gameHeight + 4 << ";1H";
#endif
        std::string controls(width, ' ');
        const std::string controlsText = gameOver ? "" : "W: Up  S: Down  M: Main Menu";
        for (int column = 0; column < width && column < static_cast<int>(controlsText.length()); ++column)
        {
            controls[column] = controlsText[column];
        }
        std::cout << controls;

#ifdef _WIN32
        COORD cursorPosition = {0, static_cast<SHORT>(gameHeight + 4)};
        SetConsoleCursorPosition(console, cursorPosition);
#else
        std::cout << "\x1B[" << gameHeight + 5 << ";1H";
#endif
        std::cout.flush();

#ifdef _WIN32
        Sleep(50);
#else
        usleep(50000);
#endif
        }
    }

#ifndef _WIN32
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

    tcsetattr(STDIN_FILENO, TCSANOW, &originalTerminal);
    std::cout << "\x1B[?25h";
    std::cout.flush();
#else
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));

    if (cursorInfoSaved)
    {
        SetConsoleCursorInfo(console, &originalCursorInfo);
    }
#endif
}

int main()
{
    char choice = '\0';
    int bestScore = 0;
    std::ifstream bestScoreFile("best_score.dat");
    int loadedBestScore = 0;
    std::string extraValue;
    if (bestScoreFile >> loadedBestScore &&
        loadedBestScore >= 0 &&
        !(bestScoreFile >> extraValue))
    {
        bestScore = loadedBestScore;
    }

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
            showTerminalRacer(bestScore);
            choice = '\0';
        }
    }

    return 0;
}
