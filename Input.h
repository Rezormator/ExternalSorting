#pragma once

#include <vector>
#include <string>

class Input
{
public:
    static int indexMenu(std::vector<std::string> options, bool addExit = false, const std::string & message = "");
    static int inputInt(std::string message);
};
