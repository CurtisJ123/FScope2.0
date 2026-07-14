#pragma once
#include <string>
#include <iostream>

class TerminalInput {
public:
    TerminalInput();
    char GetInput();
    bool isValidInput(char input);
};