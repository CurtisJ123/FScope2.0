#include "TerminalInput.h"
#include <cctype>

TerminalInput::TerminalInput() = default;

char TerminalInput::readCharacter() {
    char input{};
    std::cin >> input;
    return input;
}

int TerminalInput::readNumber() {
    int input{};

    if (std::cin >> input) {
        return input;
    }

    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return -1;
}

bool TerminalInput::readLine(std::string& line){
    return static_cast<bool>(std::getline(std::cin, line));
}

bool TerminalInput::isAlphanumericInput(char input) {
    return std::isalnum(static_cast<unsigned char>(input)) != 0;
}

std::string TerminalInput::readPath(){
    std::string path;
    std::getline(std::cin >> std::ws, path);
    return path;
}
