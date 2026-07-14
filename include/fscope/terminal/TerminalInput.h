#pragma once
#include <string>
#include <iostream>

class TerminalInput {
public:
    TerminalInput();
    char getInput();
    bool isValidInput(char input);
};