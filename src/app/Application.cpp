#include "Application.h"

Application::Application()
    : terminalDisplay(), terminalInput(), fileSystem() {}

void Application::run(){
    fileSystem.initializeDrives();
    terminalDisplay.clear();
    for(auto& drive : fileSystem.drives){
        terminalDisplay.appendDisplayText(drive->name);
        terminalDisplay.appendDisplayText(" \n");
    }
    terminalDisplay.display();


    terminalDisplay.setDisplayText("Please enter a file path to view its size\n");
    terminalDisplay.display();
    std::string path = terminalInput.getPathInput();
    FileEntry* entry = new FileEntry();
    entry->path = std::filesystem::path(path);
    fileSystem.getFileSize(entry);
    terminalDisplay.setDisplayText(std::to_string(entry->size));
    terminalDisplay.display();
}
