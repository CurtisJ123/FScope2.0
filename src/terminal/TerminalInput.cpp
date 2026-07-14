#include "TerminalInput.h"

TerminalInput::TerminalInput() = default;

char TerminalInput::GetInput() {
    char input;
    while (true) {
        std::cin >> input;
        if (isValidInput(input)) return input;
    }
}

bool TerminalInput::isValidInput(char input) {
    return isalnum(input);
}