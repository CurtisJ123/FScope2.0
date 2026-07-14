#pragma once
#include <iostream>
#include <string>

class TerminalDisplay {
private:
    std::string displayText;

public:
    TerminalDisplay();
    void display() const;
    void setDisplayText(const std::string& dt);
    void setDisplayText();
    void clear();
    void appendDisplayText(const std::string& at);
    
};