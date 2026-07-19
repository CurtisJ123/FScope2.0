#pragma once
#include <iostream>
#include <string>
#include <ViewState.h>
#include <FileSystem.h>
#include <array>
#include <format>
#include <string_view>
#include <vector>
#include <algorithm>


class TerminalDisplay {
private:
    std::string displayText;
    

public:
    TerminalDisplay();


    void display() const;
    void display(const FileSystem& fileSystem, const ViewState& view);
    void setBufferText(const std::string& dt);
    void setDisplayText();
    void clearBuffer();
    void appendText(const std::string& at);
    std::string formatSize(std::uintmax_t bytes) const;
    void appendText(const int& number);
    
};
