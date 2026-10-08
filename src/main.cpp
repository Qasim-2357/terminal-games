#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include "racer.h"
#include "snake.h"
#include "terminal.h"

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
        terminal::printColored(menuLine("2  SNAKE", 3) + "\n", "\x1B[94m");
        terminal::printColored(menuLine("3  COMING SOON...", 3) + "\n", "\x1B[90m");
        terminal::printColored(menuLine("Q  QUIT", 3) + "\n", "\x1B[90m");
        terminal::printColored(std::string(menuWidth, ' ') + "\n", "\x1B[0m");
        terminal::printColored(border + "\n", "\x1B[90m");
        terminal::printColored("Choose an option: ", "\x1B[90m");

        std::cin >> choice;

        if (choice == '1')
        {
            if (!runTerminalRacer(bestScore))
            {
                choice = 'q';
            }
            else
            {
                choice = '\0';
            }
        }
        else if (choice == '2')
        {
            if (!runSnake(bestScore))
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
