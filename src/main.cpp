#include <iostream>

int main()
{
    char choice = '\0';

    while (choice != 'q' && choice != 'Q')
    {
        std::cout << "\nTERMINAL GAMES\n"
                  << "1. Terminal Racer\n"
                  << "2. Coming Soon...\n"
                  << "Q. Quit\n"
                  << "\nChoose an option: ";

        std::cin >> choice;
    }

    return 0;
}
