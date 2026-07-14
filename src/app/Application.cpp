#include "Application.h"

Application::Application()
    : terminalDisplay(), terminalInput() {}

void Application::run(){
    terminalDisplay.setDisplayText("Welcome to FScope 2.0 Please select your drive to begin!\n");
    terminalDisplay.Display();

    
    while(true){
        char input = terminalInput.GetInput();
        if(input == '1'){
            terminalDisplay.setDisplayText("You've selected one");
            terminalDisplay.Display();
            break;
        }else{
            terminalDisplay.setDisplayText("Wrong selection");
            terminalDisplay.Display();
        }
    }
}
