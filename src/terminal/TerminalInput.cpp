#include "TerminalInput.h"


TerminalInput::TerminalInput() = default;

char TerminalInput::readCharacter() {
    char input;
    std::cin >> input;
    return input;
}

int TerminalInput::readNumber() {
    int input;

    if (std::cin >> input) {
        return input;
    }

    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return -1;
}

std::string TerminalInput::readLine(){
    std::string line;
    std::getline(std::cin, line);
    return line;
}

bool TerminalInput::isAlphanumericInput(char input) {
    return isalnum(input);
}

std::string TerminalInput::readPath(){
    std::string path;
    std::getline(std::cin >> std::ws, path);
    return path;
}
