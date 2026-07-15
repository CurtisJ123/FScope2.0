#include "TerminalInput.h"

TerminalInput::TerminalInput() = default;

char TerminalInput::getInput() {
    char input;
    std::cin >> input;
    return input;
}

bool TerminalInput::isValidInput(char input) {
    return isalnum(input);
}

std::string TerminalInput::getPathInput(){
    std::string path;
    std::getline(std::cin >> std::ws, path);
    return path;
}
