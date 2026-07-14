#include "TerminalDisplay.h"

TerminalDisplay::TerminalDisplay()
    : displayText() {}

void TerminalDisplay::Display() const {
    std::cout << displayText;
}

void TerminalDisplay::setDisplayText(const std::string& dt) {
    displayText = dt;
}