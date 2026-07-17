#pragma once
#include <iostream>
#include <string>
#include <ViewState.h>

class TerminalDisplay {
private:
    std::string displayText;
    

public:
    TerminalDisplay();

    FileSystem* fileSystem;

    void display() const;
    void display(ViewState& view);
    void setDisplayText(const std::string& dt);
    void setDisplayText();
    void clear();
    void appendDisplayText(const std::string& at);
    
};