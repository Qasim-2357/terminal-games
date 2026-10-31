#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "snake.h"
#include "terminal.h"

namespace
{
struct Fruit
{
    Point position;
    char symbol;
    int value;
};
}

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
    terminalSession.flushInput();

    auto render = [&](const std::vector<Point>& snake,
                      const std::vector<Fruit>& foods,
                      bool specialActive, const Point& specialFood,
                      int score, const char* state, bool drawHud)
    {
        (void)drawHud;
        std::vector<std::string> board(
            boardHeight, std::string(boardWidth, ' '));
        board.front().assign(boardWidth, '=');
        board.back().assign(boardWidth, '=');
        for (int row = 1; row < boardHeight - 1; ++row)
        {
            board[row].front() = '|';
            board[row].back() = '|';
        }

        for (const Fruit& food : foods)
        {
            board[food.position.y][food.position.x] = food.symbol;
        }
        if (specialActive)
        {
            board[specialFood.y][specialFood.x] = '$';
        }
        for (std::size_t segment = 0; segment < snake.size(); ++segment)
        {
            const Point& part = snake[segment];
            board[part.y][part.x] =
                segment + 1 == snake.size() ? '@' : 'o';
        }

        const bool showEndPopup =
            std::string(state) == "GAME OVER" || std::string(state) == "YOU WIN";
        const int popupRow = (boardHeight - 5) / 2;
        const std::string popupLines[5] = {
            std::string(state) == "YOU WIN" ? "YOU WIN" : "GAME OVER",
            "SCORE " + std::to_string(score),
            "BEST " + std::to_string(bestScore),
            "R  PLAY AGAIN",
            "M  MENU    Q  QUIT"
        };
        std::size_t popupWidth = 0;
        for (const std::string& line : popupLines)
        {
            popupWidth = (std::max)(popupWidth, line.size());
        }
        const int popupLeft = (boardWidth - static_cast<int>(popupWidth)) / 2;
        if (showEndPopup)
        {
            for (int line = 0; line < 5; ++line)
            {
                const std::string& message = popupLines[line];
                if (popupLeft >= 0 &&
                    popupLeft + static_cast<int>(popupWidth) <= boardWidth)
                {
                    board[popupRow + line].replace(popupLeft, message.size(), message);
                }
            }
        }

        auto writeRow = [&](int row, const std::string& text,
                            const char* color)
        {
            const std::string visibleText =
                text.substr(0, static_cast<std::size_t>(terminalWidth));
            terminal::moveCursor(row, 0);
            terminal::printColored(
                visibleText + std::string(
                           static_cast<std::size_t>(
                               (std::max)(0, terminalWidth -
                                             static_cast<int>(
                                                 visibleText.size()))),
                           ' '),
                color);
        };
        if (drawHud)
        {
            writeRow(0, "SNAKE", "\x1B[96m");
            writeRow(1, "SCORE: " + std::to_string(score) +
                           "    BEST: " + std::to_string(bestScore),
                     "\x1B[97m");
        }
        for (int rowIndex = 0; rowIndex < boardHeight; ++rowIndex)
        {
            std::string row = board[rowIndex];
            row.append(static_cast<std::size_t>(terminalWidth - boardWidth), ' ');
            terminal::moveCursor(rowIndex + 2, 0);
            std::size_t column = 0;
            while (column < row.size())
            {
                auto colorFor = [&](std::size_t position)
                {
                    const int popupLine = rowIndex - popupRow;
                    if (showEndPopup && popupLine >= 0 && popupLine < 5 &&
                        position >= static_cast<std::size_t>(popupLeft) &&
                        position < static_cast<std::size_t>(
                                      popupLeft + popupLines[popupLine].size()))
                    {
                        return 7 + popupLine;
                    }
                    const char cell = row[position];
                    return cell == 'O' ? 1 : (cell == '*' ? 2 :
                           (cell == '+' ? 3 : (cell == '$' ? 4 :
                           (cell == '@' ? 5 :
                           (cell == '=' || cell == '|' ? 6 : 0)))));
                };
                const int colorKind = colorFor(column);
                const char* color = "\x1B[92m";
                switch (colorKind)
                {
                case 1: case 7: color = "\x1B[91m"; break;
                case 2: color = "\x1B[95m"; break;
                case 3: color = "\x1B[96m"; break;
                case 4: case 9: color = "\x1B[93m"; break;
                case 5: case 8: color = "\x1B[97m"; break;
                case 6: color = "\x1B[92m"; break;
                case 10: case 11: color = "\x1B[90m"; break;
                default: break;
                }
                std::size_t end = column + 1;
                while (end < row.size() && colorFor(end) == colorKind)
                {
                    ++end;
                }
                terminal::printColored(row.substr(column, end - column), color);
                column = end;
            }
        }

        std::string footer;
        if (std::string(state) == "READY")
        {
            footer = "O/*/+ FRUIT   $ GOLDEN   ANY KEY START   M MENU   Q QUIT";
        }
        else if (std::string(state) == "PAUSED")
        {
            footer = "PAUSED   P RESUME   M MENU   Q QUIT";
        }
        else if (std::string(state) == "GAME OVER")
        {
            footer = "GAME OVER   R RESTART   M MENU   Q QUIT";
        }
        else if (std::string(state) == "YOU WIN")
        {
            footer = "YOU WIN - BOARD FULL   R RESTART   M MENU   Q QUIT";
        }
        else
        {
            footer = "W/A/S/D MOVE   P PAUSE   M MENU   Q QUIT";
        }
        writeRow(boardHeight + 2, footer,
                 std::string(state) == "GAME OVER" ? "\x1B[91m" :
                 (std::string(state) == "PAUSED" ||
                          std::string(state) == "READY" ||
                          std::string(state) == "YOU WIN" ?
                      "\x1B[93m" :
                      "\x1B[90m"));
        std::cout.flush();
    };

    auto samePoint = [](Point first, Point second)
    {
        return first.x == second.x && first.y == second.y;
    };

    auto spawnGameFood = [&](const std::vector<Point>& snake,
                             const std::vector<Fruit>& foods,
                             bool specialActive, const Point& specialFood,
                             Point& food)
    {
        std::vector<Point> interiorSnake = snake;
        for (Point& part : interiorSnake)
        {
            --part.x;
            --part.y;
        }
        for (const Fruit& existingFood : foods)
        {
            interiorSnake.push_back(
                {existingFood.position.x - 1, existingFood.position.y - 1});
        }
        if (specialActive)
        {
            interiorSnake.push_back(
                {specialFood.x - 1, specialFood.y - 1});
        }

        Point interiorFood = {};
        if (!spawnFood(boardWidth - 2, boardHeight - 2, interiorSnake,
                       randomGenerator, interiorFood))
        {
            return false;
        }
        food = {interiorFood.x + 1, interiorFood.y + 1};
        return true;
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
        Direction direction = Direction::right;
        int score = 0;
        std::vector<Fruit> foods;
        const int maximumRegularFruits = 4;
        Point specialFood = {};
        bool specialActive = false;
        Point firstFood = {};
        if (!spawnGameFood(snake, foods, false, specialFood, firstFood))
        {
            terminalSession.stop();
            terminal::clear();
            return true;
        }
        foods.push_back({firstFood, 'O', 1});
        for (int fruit = 1; fruit < maximumRegularFruits; ++fruit)
        {
            Point nextFood = {};
            if (!spawnGameFood(snake, foods, false, specialFood, nextFood))
            {
                break;
            }
            const char symbols[] = {'O', '*', '+'};
            foods.push_back({nextFood, symbols[fruit % 3], 1});
        }
        if (score % 3 == 0 &&
            spawnGameFood(snake, foods, false, specialFood, specialFood))
        {
            specialActive = true;
        }
        bool gameOver = false;
        bool won = false;
        bool paused = false;
        bool started = false;

        terminal::clear();
        render(snake, foods, specialActive, specialFood, score, "READY", true);

        auto specialAppearedAt = std::chrono::steady_clock::now();
        auto lastFrame = specialAppearedAt;
        auto pauseStartedAt = specialAppearedAt;
        while (running && !started && !gameOver)
        {
            const char input =
                terminalSession.pollInput(false, false, pauseKeyWasDown);
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
            else if (input != '\0')
            {
                started = true;
                lastFrame = std::chrono::steady_clock::now();
                specialAppearedAt = lastFrame;
                render(snake, foods, specialActive, specialFood, score,
                       "PLAYING", true);
            }
            terminal::sleepMilliseconds(10);
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
                const auto pauseChangedAt = std::chrono::steady_clock::now();
                paused = !paused;
                if (paused)
                {
                    pauseStartedAt = pauseChangedAt;
                }
                else
                {
                    const auto pausedFor = pauseChangedAt - pauseStartedAt;
                    lastFrame = pauseChangedAt;
                    if (specialActive)
                    {
                        specialAppearedAt += pausedFor;
                    }
                }
                render(snake, foods, specialActive, specialFood, score,
                       paused ? "PAUSED" : "PLAYING", true);
            }
            else if (!paused && input != '\0')
            {
                direction = directionForInput(direction, input);
            }

            if (!running)
            {
                break;
            }

            if (!paused)
            {
                const auto now = std::chrono::steady_clock::now();
                if (specialActive &&
                    now - specialAppearedAt >= std::chrono::seconds(5))
                {
                    specialActive = false;
                    render(snake, foods, specialActive, specialFood, score,
                           "PLAYING", true);
                }
                if (now - lastFrame >=
                    std::chrono::milliseconds(
                        movementIntervalMilliseconds(score)))
                {
                    lastFrame = now;
                    Point nextHead = nextPoint(snake.back(), direction);
                    const auto regularFruit = std::find_if(
                        foods.begin(), foods.end(),
                        [&](const Fruit& food)
                        {
                            return samePoint(food.position, nextHead);
                        });
                    const bool ateRegularFruit =
                        regularFruit != foods.end();
                    const bool ateSpecialFruit =
                        specialActive && samePoint(nextHead, specialFood);
                    if (nextHead.x < 1 || nextHead.x >= boardWidth - 1 ||
                        nextHead.y < 1 || nextHead.y >= boardHeight - 1 ||
                        isCollision(nextHead, snake,
                                    ateRegularFruit || ateSpecialFruit))
                    {
                        gameOver = true;
                    }
                    else
                    {
                        const int fruitValue =
                            ateSpecialFruit ? 5 : regularFruit->value;
                        snake.push_back(nextHead);
                        if (!ateRegularFruit && !ateSpecialFruit)
                        {
                            snake.erase(snake.begin());
                        }
                        else
                        {
                            score += fruitValue;
                            if (score > bestScore)
                            {
                                bestScore = score;
                                std::ofstream bestScoreOutput(
                                    "snake_best_score.dat");
                                bestScoreOutput << bestScore << '\n';
                            }
                            if (ateRegularFruit)
                            {
                                foods.erase(regularFruit);
                            }
                            if (ateSpecialFruit)
                            {
                                specialActive = false;
                            }
                            if (ateRegularFruit)
                            {
                                Point nextFood = {};
                                if (spawnGameFood(snake, foods,
                                                  specialActive, specialFood,
                                                  nextFood))
                                {
                                    const char symbols[] = {'O', '*', '+'};
                                    foods.push_back(
                                        {nextFood,
                                         symbols[score % 3],
                                         1});
                                }
                                else if (snake.size() >=
                                         static_cast<std::size_t>(
                                             (boardWidth - 2) *
                                             (boardHeight - 2)))
                                {
                                    won = true;
                                }
                            }
                            if (!specialActive && score > 0 && score % 3 == 0 &&
                                spawnGameFood(snake, foods, false, specialFood,
                                              specialFood))
                            {
                                specialActive = true;
                                specialAppearedAt = now;
                            }
                        }
                    }
                    if (won)
                    {
                        gameOver = true;
                    }
                    render(snake, foods, specialActive, specialFood, score,
                           won ? "YOU WIN" : (gameOver ? "GAME OVER" :
                                              "PLAYING"),
                           true);
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
