#pragma once
#include <string>
#include <FileEntry.h>

enum StateType{
    DriveSelection,
    Scanning,
    EntryContents,
    Error
};

enum ScanningState{
    NotStarted,
    InProgress,
    Complete,
    Partial,
    Failed
};

class ViewState
{
public:
    ViewState() = default;
    ~ViewState() = default;

    int selectedIndex = 0;
    std::string selectionInput = "";
    std::string statusMessage = "";
    ScanningState scanningState = NotStarted;
    StateType currentState = DriveSelection;
    FileEntry* selectedEntry = nullptr;


private:
};

