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
}
