#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "snake.h"
#include "terminal.h"

bool runSnake()
{
    int bestScore = 0;
    std::ifstream bestScoreFile("snake_best_score.dat");
    int loadedBestScore = 0;
    std::string extraValue;
    if (bestScoreFile >> loadedBestScore &&
        loadedBestScore >= 0 &&
        !(bestScoreFile >> extraValue))
    {
        bestScore = loadedBestScore;
    }

    int terminalWidth = 0;
    int terminalHeight = 0;
    if (!terminal::getSize(terminalWidth, terminalHeight))
    {
        return true;
    }

    const int minimumWidth = 20;
    const int minimumHeight = 10;
    if (terminalWidth < minimumWidth || terminalHeight < minimumHeight)
    {
        std::cout << "The terminal is too small to display Snake.\n"
                  << "Press M to return to the main menu: ";

        char choice = '\0';
        while (choice != 'm' && choice != 'M')
        {
            std::cin >> choice;
        }
        terminal::clear();
        return true;
    }

    const int boardWidth = terminalWidth - 2;
    const int boardHeight = terminalHeight - 4;
    std::random_device randomDevice;
    std::mt19937 randomGenerator(randomDevice());
    bool running = true;
    bool pauseKeyWasDown = false;
    terminal::Session terminalSession;
    if (!terminalSession.start())
    {
        return true;
    }

    auto spawnFood = [&](const std::vector<Point>& snake)
    {
        std::uniform_int_distribution<int> xDistribution(1, boardWidth - 2);
        std::uniform_int_distribution<int> yDistribution(1, boardHeight - 2);
        Point food;
        do
        {
            food = {xDistribution(randomGenerator), yDistribution(randomGenerator)};
        }
        while (std::any_of(snake.begin(), snake.end(),
                           [&](const Point& part)
                           {
                               return part.x == food.x && part.y == food.y;
                           }));
        return food;
    };

    auto render = [&](const std::vector<Point>& snake, const Point& food,
                      int score, const char* state, bool drawHud)
    {
        terminal::moveCursor(drawHud ? 0 : 2, 0);
        std::vector<std::string> board(
            boardHeight, std::string(boardWidth, ' '));
        board.front().assign(boardWidth, '=');
        board.back().assign(boardWidth, '=');
        for (int row = 1; row < boardHeight - 1; ++row)
        {
            board[row].front() = '|';
            board[row].back() = '|';
        }

        board[food.y][food.x] = 'O';
        for (std::size_t segment = 0; segment < snake.size(); ++segment)
        {
            const Point& part = snake[segment];
            board[part.y][part.x] =
                segment + 1 == snake.size() ? '@' : 'o';
        }

        if (drawHud)
        {
            terminal::printColored("SNAKE\n", "\x1B[96m");
            terminal::printColored(
                "SCORE: " + std::to_string(score) +
                "    BEST: " + std::to_string(bestScore) + "\n",
                "\x1B[97m");
        }
        for (int rowIndex = 0; rowIndex < boardHeight; ++rowIndex)
        {
            const std::string& row = board[rowIndex];
            const std::size_t foodPosition =
                rowIndex == food.y ? static_cast<std::size_t>(food.x) :
                                     std::string::npos;
            if (foodPosition == std::string::npos)
            {
                terminal::printColored(row + "\n", "\x1B[92m");
            }
            else
            {
                terminal::printColored(
                    row.substr(0, foodPosition), "\x1B[92m");
                terminal::printColored("O", "\x1B[91m");
                terminal::printColored(
                    row.substr(foodPosition + 1) + "\n", "\x1B[92m");
            }
        }

        if (std::string(state) == "READY")
        {
            terminal::printColored(
                "PRESS ANY KEY TO START\n"
                "M MENU    Q QUIT\n",
                "\x1B[93m");
        }
        else if (std::string(state) == "PAUSED")
        {
            terminal::printColored(
                "PAUSED\n"
                "P RESUME    M MENU    Q QUIT\n",
                "\x1B[93m");
        }
        else if (std::string(state) == "GAME OVER")
        {
            terminal::printColored(
                "GAME OVER\n"
                "R RESTART    M MENU    Q QUIT\n",
                "\x1B[91m");
        }
        else
        {
            terminal::printColored(
                "W/A/S/D MOVE    P PAUSE    M MENU    Q QUIT\n",
                "\x1B[90m");
        }
        std::cout.flush();
    };

    while (running)
    {
        const int centerX = boardWidth / 2;
        const int centerY = boardHeight / 2;
        std::vector<Point> snake = {
            {centerX - 2, centerY},
            {centerX - 1, centerY},
            {centerX, centerY}
        };
        int directionX = 1;
        int directionY = 0;
        int score = 0;
        Point food = spawnFood(snake);
        bool gameOver = false;
        bool paused = false;
        bool started = false;
        int movementElapsed = 0;

        terminal::clear();
        render(snake, food, score, "READY", true);
        while (running && !started)
        {
            const char input =
                terminalSession.pollInput(false, false, pauseKeyWasDown);
            if (input == 'm' || input == 'M')
            {
                running = false;
                terminal::clear();
            }
            else if (input == 'q' || input == 'Q')
            {
                terminalSession.stop();
                terminal::clear();
                return false;
            }
            else
            {
                started = true;
            }
            terminal::sleepMilliseconds(10);
        }

        if (running && started)
        {
            terminal::clear();
            render(snake, food, score, "PLAYING", true);
        }

        while (running && started && !gameOver)
        {
            const char input =
                terminalSession.pollInput(true, false, pauseKeyWasDown);
            if (input == 'm' || input == 'M')
            {
                running = false;
            }
            else if (input == 'q' || input == 'Q')
            {
                terminalSession.stop();
                terminal::clear();
                return false;
            }
            else if (input == 'p' || input == 'P')
            {
                paused = !paused;
                terminal::clear();
                render(snake, food, score, paused ? "PAUSED" : "PLAYING",
                       true);
            }
            else if (!paused && input != '\0')
            {
                if ((input == 'w' || input == 'W') && directionY == 0)
                {
                    directionX = 0;
                    directionY = -1;
                }
                else if ((input == 's' || input == 'S') && directionY == 0)
                {
                    directionX = 0;
                    directionY = 1;
                }
                else if ((input == 'a' || input == 'A') && directionX == 0)
                {
                    directionX = -1;
                    directionY = 0;
                }
                else if ((input == 'd' || input == 'D') && directionX == 0)
                {
                    directionX = 1;
                    directionY = 0;
                }
            }

            if (!running)
            {
                break;
            }

            if (!paused)
            {
                movementElapsed += 10;
                if (movementElapsed >= 70)
                {
                    movementElapsed -= 70;
                    Point nextHead = snake.back();
                    nextHead.x += directionX;
                    nextHead.y += directionY;
                    bool redrawHud = false;
                    if (nextHead.x < 1 || nextHead.x >= boardWidth - 1 ||
                        nextHead.y < 1 || nextHead.y >= boardHeight - 1 ||
                        std::any_of(snake.begin(), snake.end(),
                                    [&](const Point& part)
                                    {
                                        return part.x == nextHead.x &&
                                               part.y == nextHead.y;
                                    }))
                    {
                        gameOver = true;
                    }
                    else
                    {
                        const bool ateFood =
                            nextHead.x == food.x && nextHead.y == food.y;
                        snake.push_back(nextHead);
                        if (!ateFood)
                        {
                            snake.erase(snake.begin());
                        }
                        else
                        {
                            ++score;
                            redrawHud = true;
                            if (score > bestScore)
                            {
                                bestScore = score;
                                std::ofstream bestScoreOutput(
                                    "snake_best_score.dat");
                                bestScoreOutput << bestScore << '\n';
                            }
                            food = spawnFood(snake);
                        }
                    }
                    if (gameOver)
                    {
                        terminal::clear();
                    }
                    render(snake, food, score,
                           gameOver ? "GAME OVER" : "PLAYING",
                           gameOver || redrawHud);
                }
            }
            terminal::sleepMilliseconds(10);
        }

        while (running && gameOver)
        {
            const char input =
                terminalSession.pollInput(true, true, pauseKeyWasDown);
            if (input == 'r' || input == 'R')
            {
                terminalSession.flushInput();
                terminal::clear();
                break;
            }
            if (input == 'm' || input == 'M')
            {
                running = false;
                terminal::clear();
                break;
            }
            if (input == 'q' || input == 'Q')
            {
                running = false;
                terminalSession.stop();
                terminal::clear();
                return false;
            }
        }
    }

    terminalSession.stop();
    terminal::clear();
    terminal::moveCursor(0, 0);
    return true;
}
