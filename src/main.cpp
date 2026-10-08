#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include "terminal.h"

constexpr int getObstacleSpeed(int score)
{
    if (score >= 30)
    {
        return 6;
    }
    if (score >= 20)
    {
        return 5;
    }
    if (score >= 10)
    {
        return 4;
    }
    if (score >= 5)
    {
        return 3;
    }
    return 2;
}

constexpr int getObstacleCount(int score)
{
    if (score >= 20)
    {
        return 5;
    }
    if (score >= 10)
    {
        return 4;
    }
    return 3;
}

constexpr bool sweptCollision(
    int carLeft,
    int carWidth,
    int carTop,
    int obstacleLeft,
    int obstacleWidth,
    int obstacleRow,
    int obstacleSpeed)
{
    const int previousLeft = obstacleLeft;
    const int movedLeft = previousLeft - obstacleSpeed;
    const int pathLeft = movedLeft < 0 ? 0 : movedLeft;
    const bool horizontalOverlap =
        carLeft < previousLeft + obstacleWidth &&
        carLeft + carWidth > pathLeft;
    const bool verticalOverlap =
        obstacleRow >= carTop &&
        obstacleRow <= carTop + 1;

    return horizontalOverlap && verticalOverlap;
}

constexpr bool isCarWithinRoad(int carTop, int roadTop, int roadBottom)
{
    return carTop >= roadTop + 1 &&
           carTop <= roadBottom - 2;
}

static_assert(getObstacleSpeed(0) == 2);
static_assert(getObstacleSpeed(5) == 3);
static_assert(getObstacleSpeed(10) == 4);
static_assert(getObstacleSpeed(20) == 5);
static_assert(getObstacleSpeed(30) == 6);
static_assert(getObstacleCount(0) == 3);
static_assert(getObstacleCount(10) == 4);
static_assert(getObstacleCount(20) == 5);
static_assert(isCarWithinRoad(1, 0, 11));
static_assert(isCarWithinRoad(9, 0, 11));
static_assert(!isCarWithinRoad(0, 0, 11));
static_assert(!isCarWithinRoad(10, 0, 11));
static_assert(sweptCollision(10, 7, 5, 20, 3, 5, 10));
static_assert(!sweptCollision(10, 7, 5, 20, 3, 1, 10));

void showTerminalRacer(int& bestScore, bool& quitRequested)
{
    int width = 0;
    int height = 0;

    if (!terminal::getSize(width, height))
    {
        std::cout << "Could not determine the terminal dimensions.\n"
                  << "Press M to return to the main menu: ";

        char choice = '\0';
        while (choice != 'm' && choice != 'M')
        {
            std::cin >> choice;
        }
        terminal::clear();
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
        terminal::clear();
        return;
    }

    const int carWidth = 7;
    const int carLeft = (width - carWidth) / 2;
    const int visualCarWidth = 18;
    const int visualCarLeft = carLeft - (visualCarWidth - carWidth) / 2;
    const int minimumCarTop = roadTop + 1;
    const int maximumCarTop = roadBottom - 2;
    const int obstacleWidth = 3;
    const int obstaclePatterns[5][8] = {
        {roadTop + 1, roadTop + 1, roadTop + 5, roadTop + 5,
         roadTop + 9, roadTop + 9, roadTop + 4, roadTop + 7},
        {roadTop + 9, roadTop + 9, roadTop + 5, roadTop + 5,
         roadTop + 1, roadTop + 1, roadTop + 6, roadTop + 3},
        {roadTop + 1, roadTop + 2, roadTop + 3, roadTop + 5,
         roadTop + 6, roadTop + 7, roadTop + 4, roadTop + 8},
        {roadTop + 8, roadTop + 8, roadTop + 4, roadTop + 4,
         roadTop + 2, roadTop + 2, roadTop + 6, roadTop + 9},
        {roadTop + 2, roadTop + 3, roadTop + 4, roadTop + 7,
         roadTop + 8, roadTop + 9, roadTop + 5, roadTop + 1}
    };

    terminal::Session terminalSession;
    if (!terminalSession.start())
    {
        return;
    }
    terminal::clear();

    bool playing = true;
    while (playing)
    {
        int carTop = maximumCarTop;
        const int obstacleSpacing = width / 12 > 5 ? width / 12 : 6;
        int obstacleLeft[8] = {
            width + obstacleSpacing,
            width + obstacleSpacing,
            width + obstacleSpacing * 2,
            width + obstacleSpacing * 3,
            width + obstacleSpacing * 4,
            width + obstacleSpacing * 5,
            width + obstacleSpacing * 6,
            width + obstacleSpacing * 7
        };
        int obstacleRow[8] = {
            obstaclePatterns[0][0],
            obstaclePatterns[0][1],
            obstaclePatterns[0][2],
            obstaclePatterns[0][3],
            obstaclePatterns[0][4],
            obstaclePatterns[0][5],
            obstaclePatterns[0][6],
            obstaclePatterns[0][7]
        };
        int score = 0;
        int activeObstacleCount = 3;
        bool gameOver = false;
        bool started = false;
        bool paused = false;
        bool pauseKeyWasDown = false;
        bool firstFrame = true;

        while (playing)
        {
        char input = terminalSession.pollInput(started, gameOver, pauseKeyWasDown);

        if (!started)
        {
            if (input == 'm' || input == 'M')
            {
                playing = false;
            }
            else if (input == 'q' || input == 'Q')
            {
                playing = false;
                quitRequested = true;
            }
            else if (input != '\0' && input != 'r' && input != 'R')
            {
                started = true;
            }
        }
        else if (input == 'm' || input == 'M')
        {
            playing = false;
        }
        else if (input == 'q' || input == 'Q')
        {
            playing = false;
            quitRequested = true;
        }
        else if (gameOver && (input == 'r' || input == 'R'))
        {
            break;
        }
        else if (!gameOver && (input == 'p' || input == 'P'))
        {
            paused = !paused;
        }
        else if (!gameOver && !paused &&
                 (input == 'w' || input == 'W') && carTop > minimumCarTop)
        {
            --carTop;
        }
        else if (!gameOver && !paused &&
                 (input == 's' || input == 'S') && carTop < maximumCarTop)
        {
            ++carTop;
        }

        if (!playing)
        {
            break;
        }

        if (started && !paused && !gameOver)
        {
            const int obstacleSpeed = getObstacleSpeed(score);

            for (int obstacle = 0; obstacle < activeObstacleCount; ++obstacle)
            {
                if (sweptCollision(
                        carLeft,
                        carWidth,
                        carTop,
                        obstacleLeft[obstacle],
                        obstacleWidth,
                        obstacleRow[obstacle],
                        obstacleSpeed))
                {
                    gameOver = true;
                }

                obstacleLeft[obstacle] -= obstacleSpeed;
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
                    const int pattern = (score / 5) % 5;
                    obstacleRow[obstacle] =
                        obstaclePatterns[pattern][obstacle];
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

            const int desiredObstacleCount = getObstacleCount(score);
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
                const int pattern = (score / 5) % 5;
                obstacleRow[activeObstacleCount] =
                    obstaclePatterns[pattern][activeObstacleCount];
                ++activeObstacleCount;
            }
        }

        if (!firstFrame)
        {
            terminal::moveCursor(0, 0);
        }
        firstFrame = false;

        const std::string titleText = "TERMINAL RACER";
        const std::string scoreText = "SCORE " + std::to_string(score);
        const std::string bestText = "BEST " + std::to_string(bestScore);
        const int titleColumn = width >= static_cast<int>(titleText.length()) + 2 ? 2 : 0;
        const int bestColumn = width - static_cast<int>(bestText.length()) - 2;
        const int scoreColumn =
            bestColumn - static_cast<int>(scoreText.length()) - 4;

        terminal::moveCursor(0, 0);
        terminal::printColored(std::string(width, ' '), "\x1B[0m");
        if (titleColumn + static_cast<int>(titleText.length()) <= width)
        {
            terminal::moveCursor(0, titleColumn);
            terminal::printColored(titleText, "\x1B[96m");
        }
        if (scoreColumn >= 0 && scoreColumn + static_cast<int>(scoreText.length()) <= width)
        {
            terminal::moveCursor(0, scoreColumn);
            terminal::printColored(scoreText, "\x1B[97m");
        }
        if (bestColumn >= 0 && bestColumn + static_cast<int>(bestText.length()) <= width)
        {
            terminal::moveCursor(0, bestColumn);
            terminal::printColored(bestText, "\x1B[93m");
        }

        terminal::moveCursor(1, 0);
        terminal::printColored(std::string(width, ' '), "\x1B[0m");

        terminal::moveCursor(2, 0);
        terminal::printColored(std::string(width, '-'), "\x1B[90m");
        std::cout.flush();

        for (int row = 0; row < gameHeight; ++row)
        {
            std::string line(width, ' ');
            const bool roadBorder = row == roadTop || row == roadBottom;
            const bool laneMarkingRow =
                row > roadTop && row < roadBottom &&
                (row == roadTop + 3 || row == roadTop + 6 || row == roadTop + 9);

            if (roadBorder)
            {
                line.assign(width, '=');
                if (width >= 2)
                {
                    line.front() = '+';
                    line.back() = '+';
                }
            }
            else
            {
                if (width >= 2)
                {
                    line[1] = '|';
                    line[width - 2] = '|';
                }

                if (laneMarkingRow)
                {
                    const int markingPhase = obstacleLeft[0];
                    for (int column = 2; column < width - 2; ++column)
                    {
                        if ((column + markingPhase) % 12 < 5)
                        {
                            line[column] = '-';
                        }
                    }
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
                            line[obstacleColumn] = '[';
                            if (column == 1)
                            {
                                line[obstacleColumn] = '#';
                            }
                            else if (column == 2)
                            {
                                line[obstacleColumn] = ']';
                            }
                        }
                    }
                }
            }

            if (!gameOver && row == carTop &&
                visualCarLeft >= 0 && visualCarLeft + visualCarWidth <= width)
            {
                line.replace(visualCarLeft, visualCarWidth, "<____/\\___________");
            }
            else if (!gameOver && row == carTop + 1 &&
                     visualCarLeft >= 0 && visualCarLeft + visualCarWidth <= width)
            {
                line.replace(visualCarLeft, visualCarWidth, "O================O");
            }

            bool popupLine = false;
            const char* popupColor = "\x1B[91m";
            if (gameOver)
            {
                const int popupRow = (roadTop + roadBottom) / 2 - 2;
                std::string gameOverText;

                if (row == popupRow)
                {
                    gameOverText = "GAME OVER";
                    popupColor = "\x1B[91m";
                }
                else if (row == popupRow + 1)
                {
                    gameOverText = "SCORE " + std::to_string(score);
                    popupColor = "\x1B[97m";
                }
                else if (row == popupRow + 2)
                {
                    gameOverText = "BEST " + std::to_string(bestScore);
                    popupColor = "\x1B[93m";
                }
                else if (row == popupRow + 3)
                {
                    gameOverText = "R  PLAY AGAIN";
                    popupColor = "\x1B[90m";
                }
                else if (row == popupRow + 4)
                {
                    gameOverText = "M  MENU    Q  QUIT";
                    popupColor = "\x1B[90m";
                }

                if (!gameOverText.empty())
                {
                    popupLine = true;
                    const int textLeft = (width - static_cast<int>(gameOverText.length())) / 2;
                    if (textLeft >= 0 && textLeft + static_cast<int>(gameOverText.length()) <= width)
                    {
                        line.replace(textLeft, gameOverText.length(), gameOverText);
                    }
                }
            }
            else if (!started && row == (roadTop + roadBottom) / 2)
            {
                const std::string startText = "READY?";
                const int textLeft = (width - static_cast<int>(startText.length())) / 2;
                if (textLeft >= 0 && textLeft + static_cast<int>(startText.length()) <= width)
                {
                    line.replace(textLeft, startText.length(), startText);
                }
            }
            else if (!started && row == (roadTop + roadBottom) / 2 + 1)
            {
                const std::string startText = "PRESS ANY KEY TO START";
                const int textLeft = (width - static_cast<int>(startText.length())) / 2;
                if (textLeft >= 0 && textLeft + static_cast<int>(startText.length()) <= width)
                {
                    line.replace(textLeft, startText.length(), startText);
                }
            }
            else if (started && paused && row == (roadTop + roadBottom) / 2)
            {
                const std::string pauseText = "PAUSED";
                const int textLeft = (width - static_cast<int>(pauseText.length())) / 2;
                if (textLeft >= 0 && textLeft + static_cast<int>(pauseText.length()) <= width)
                {
                    line.replace(textLeft, pauseText.length(), pauseText);
                }
            }
            else if (started && paused && row == (roadTop + roadBottom) / 2 + 1)
            {
                const std::string pauseText = "P  RESUME    M  MENU    Q  QUIT";
                const int textLeft = (width - static_cast<int>(pauseText.length())) / 2;
                if (textLeft >= 0 && textLeft + static_cast<int>(pauseText.length()) <= width)
                {
                    line.replace(textLeft, pauseText.length(), pauseText);
                }
            }
            terminal::moveCursor(row + 3, 0);
            const char* rowColor = "\x1B[0m";
            if (popupLine)
            {
                rowColor = popupColor;
            }
            else if (started && paused &&
                     (row == (roadTop + roadBottom) / 2 ||
                      row == (roadTop + roadBottom) / 2 + 1))
            {
                rowColor = "\x1B[93m";
            }
            else if (!gameOver && (row == carTop || row == carTop + 1))
            {
                rowColor = "\x1B[94m";
            }
            else
            {
                bool obstacleOnRow = false;
                for (int obstacle = 0; obstacle < activeObstacleCount; ++obstacle)
                {
                    if (row == obstacleRow[obstacle])
                    {
                        obstacleOnRow = true;
                        break;
                    }
                }

                if (obstacleOnRow)
                {
                    rowColor = "\x1B[91m";
                }
                else if (roadBorder || laneMarkingRow)
                {
                    rowColor = "\x1B[90m";
                }
            }

            terminal::printColored(line, rowColor);
            std::cout.flush();
        }

        terminal::moveCursor(gameHeight + 3, 0);
        std::string controls(width, ' ');
        const std::string controlsText =
            gameOver ? "R PLAY AGAIN    M MENU    Q QUIT" :
            (!started ? "M MENU    Q QUIT" :
             (paused ? "P RESUME    M MENU    Q QUIT" :
              "W/S MOVE    P PAUSE    M MENU    Q QUIT"));
        const int controlsColumn =
            (width - static_cast<int>(controlsText.length())) / 2;
        const int visibleControlsColumn = controlsColumn > 0 ? controlsColumn : 0;
        for (int column = 0; column < static_cast<int>(controlsText.length()) &&
                            visibleControlsColumn + column < width;
             ++column)
        {
            controls[visibleControlsColumn + column] = controlsText[column];
        }
        terminal::printColored(controls, "\x1B[90m");

        terminal::moveCursor(gameHeight + 4, 0);
        std::cout.flush();

        terminal::sleepMilliseconds(50);
        }
        
        if (playing)
        {
            terminalSession.flushInput();
        }
    }

    terminalSession.stop();
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
        terminal::clear();
        int menuWidth = 60;
        int menuHeight = 24;
        terminal::getSize(menuWidth, menuHeight);
        (void)menuHeight;
        if (menuWidth < 32)
        {
            menuWidth = 32;
        }

        const std::string border = "+" + std::string(menuWidth - 2, '=') + "+";
        auto menuLine = [&](const std::string& text, int leftPadding)
        {
            std::string line(menuWidth, ' ');
            if (leftPadding < 0)
            {
                leftPadding = 0;
            }
            const int start = leftPadding + 1;
            if (start < menuWidth - 1)
            {
                const int available = menuWidth - 1 - start;
                line.replace(start, (std::min)(available, static_cast<int>(text.length())),
                             text.substr(0, available));
            }
            return line;
        };
        auto centeredMenuLine = [&](const std::string& text)
        {
            return menuLine(text, (menuWidth - static_cast<int>(text.length())) / 2 - 1);
        };

        terminal::printColored(border + "\n", "\x1B[90m");
        terminal::printColored(centeredMenuLine("TERMINAL GAMES") + "\n", "\x1B[96m");
        terminal::printColored(std::string(menuWidth, ' ') + "\n", "\x1B[0m");
        terminal::printColored(menuLine("1  TERMINAL RACER", 3) + "\n", "\x1B[94m");
        terminal::printColored(menuLine("2  COMING SOON...", 3) + "\n", "\x1B[90m");
        terminal::printColored(menuLine("Q  QUIT", 3) + "\n", "\x1B[90m");
        terminal::printColored(std::string(menuWidth, ' ') + "\n", "\x1B[0m");
        terminal::printColored(border + "\n", "\x1B[90m");
        terminal::printColored("Choose an option: ", "\x1B[90m");

        std::cin >> choice;

        if (choice == '1')
        {
            bool quitRequested = false;
            showTerminalRacer(bestScore, quitRequested);
            if (quitRequested)
            {
                choice = 'q';
            }
            else
            {
                choice = '\0';
            }
        }
    }

    return 0;
}
