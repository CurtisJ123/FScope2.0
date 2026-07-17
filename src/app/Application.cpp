#include "Application.h"

Application::Application()
    : terminalDisplay(), terminalInput(), fileSystem() {}

void Application::run(){
    fileSystem.initializeDrives();
    terminalDisplay.clear();
    terminalDisplay.fileSystem = &fileSystem;
    int i = 0;
    for(auto& drive : fileSystem.drives){
        terminalDisplay.appendDisplayText(std::to_string(i));
        terminalDisplay.appendDisplayText(drive->path.string());
        terminalDisplay.appendDisplayText(" \n");
        i++;
    }
    terminalDisplay.display();
    

    terminalDisplay.setDisplayText("Please enter a number of which drive you want size of\n");
    terminalDisplay.display();
    


    char input = terminalInput.getInput();
    int inputNum = input - '0';
    auto& selectedDrive = fileSystem.drives[inputNum];

    fileSystem.getFileSize(selectedDrive.get());
    terminalDisplay.setDisplayText(std::to_string(selectedDrive->size));
    terminalDisplay.display();
}
