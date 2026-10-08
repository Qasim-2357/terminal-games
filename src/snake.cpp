#include <iostream>
#include <string>
#include <vector>

#include "snake.h"
#include "terminal.h"

bool runSnake(int& bestScore)
{
    (void)bestScore;
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
    const int centerX = boardWidth / 2;
    const int centerY = boardHeight / 2;
    std::vector<Point> snake = {
        {centerX - 2, centerY},
        {centerX - 1, centerY},
        {centerX, centerY}
    };

    int directionX = 1;
    int directionY = 0;
    bool running = true;
    bool pauseKeyWasDown = false;
    terminal::Session terminalSession;
    if (!terminalSession.start())
    {
        return true;
    }

    while (running)
    {
        const char input =
            terminalSession.pollInput(true, false, pauseKeyWasDown);
        if (input == 'm' || input == 'M')
        {
            running = false;
        }
        else if (input == 'q' || input == 'Q')
        {
            running = false;
            terminalSession.stop();
            terminal::clear();
            return false;
        }
        else if ((input == 'w' || input == 'W') && directionY == 0)
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

        if (!running)
        {
            break;
        }

        Point nextHead = snake.back();
        nextHead.x += directionX;
        nextHead.y += directionY;
        if (nextHead.x < 1 || nextHead.x >= boardWidth - 1)
        {
            nextHead.x = snake.back().x;
        }
        if (nextHead.y < 1 || nextHead.y >= boardHeight - 1)
        {
            nextHead.y = snake.back().y;
        }
        snake.push_back(nextHead);
        snake.erase(snake.begin());

        terminal::clear();
        std::vector<std::string> board(
            boardHeight, std::string(boardWidth, ' '));
        board.front().assign(boardWidth, '=');
        board.back().assign(boardWidth, '=');
        for (int row = 1; row < boardHeight - 1; ++row)
        {
            board[row].front() = '|';
            board[row].back() = '|';
        }

        for (std::size_t segment = 0; segment < snake.size(); ++segment)
        {
            const Point& part = snake[segment];
            if (part.x > 0 && part.x < boardWidth - 1 &&
                part.y > 0 && part.y < boardHeight - 1)
            {
                board[part.y][part.x] = segment + 1 == snake.size() ? '@' : 'o';
            }
        }

        terminal::printColored("SNAKE\n", "\x1B[96m");
        for (const std::string& row : board)
        {
            terminal::printColored(row + "\n", "\x1B[92m");
        }
        terminal::printColored(
            "W/A/S/D MOVE    M MENU    Q QUIT\n", "\x1B[90m");
        std::cout.flush();
        terminal::sleepMilliseconds(180);
    }

    terminalSession.stop();
    terminal::clear();
    return true;
}
