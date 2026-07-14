#pragma once
#include <iostream>
#include <string>

class TerminalDisplay {
private:
    std::string displayText;

public:
    TerminalDisplay();
    void Display() const;
    void setDisplayText(const std::string& dt);
};