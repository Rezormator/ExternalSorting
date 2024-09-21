#include <iostream>
#include <conio.h>
#include "Input.h"

int Input::indexMenu(std::vector<std::string> options, const bool addExit, const std::string &message) {
    bool continueSelection = true;
    int selectedIndex = 0;

    if (addExit)
        options.emplace_back("Exit");

    int const optionsAmount = options.size();

    do {
        system("cls");

        std::cout << message;

        for (int i = 0; i < optionsAmount; i++)
            std::cout << (i == selectedIndex ? " " : "") << options.at(i) << std::endl;

        switch (_getch()) {
            case 72:
                selectedIndex = (selectedIndex - 1 + optionsAmount) % optionsAmount;
                break;
            case 80:
                selectedIndex = (selectedIndex + 1) % optionsAmount;
                break;
            case 13:
                continueSelection = false;
                break;
            default:
                break;
        }
    } while (continueSelection);

    system("cls");

    return selectedIndex;
}

int Input::inputInt(std::string message) {
    int inputInt;
    std::cout << message;
    std::cin >> inputInt;
    return inputInt;
}
