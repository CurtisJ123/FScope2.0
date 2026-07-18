#pragma once
#include <string>
#include <iostream>
#include <limits>

class TerminalInput {
public:
    TerminalInput();
    char readCharacter();
    int readNumber();
    bool isAlphanumericInput(char input);
    std::string readPath();
    std::string readLine();
};
