#pragma once
#include <string>

enum StateType{
    DriveSelection,
    Scanning,
    DriveContents,
    Error
};

enum ScanningState{
    Complete,
    Partial,
    Error
};

class ViewState
{
public:
    ViewState() = default;
    ~ViewState() = default;

    int selectedIndex;
    std::string statusMessage;
    ScanningState scanningState;
    StateType currentState;


private:
};

