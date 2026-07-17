#include "TerminalDisplay.h"

TerminalDisplay::TerminalDisplay()
    : displayText() {}

void TerminalDisplay::display() const {
    std::cout << displayText;
}

void TerminalDisplay::setDisplayText(const std::string& dt) {
    displayText = dt;
}

void TerminalDisplay::setDisplayText() {
    displayText = "";
}

void TerminalDisplay::clear(){
    displayText = "";
}

void TerminalDisplay::appendDisplayText(const std::string& at){
    displayText += at;
}

void TerminalDisplay::display(ViewState& view){
    switch(view.currentState){
        case StateType::DriveSelection:
            break;
        case StateType::Scanning:
            break;
        case StateType::DriveContents:
            break;
        case StateType::Error:
            break;
    }
}