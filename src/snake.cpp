#include <iostream>

#include "snake.h"
#include "terminal.h"

bool runSnake(int& bestScore)
{
    (void)bestScore;
    terminal::clear();
    terminal::printColored("SNAKE\n\n", "\x1B[96m");
    terminal::printColored("Snake gameplay is coming soon.\n", "\x1B[90m");
    terminal::printColored("Press M to return to the main menu: ", "\x1B[90m");

    char choice = '\0';
    while (choice != 'm' && choice != 'M')
    {
        std::cin >> choice;
    }

    terminal::clear();
    return true;
}
