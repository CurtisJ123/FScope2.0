#pragma once

#include "TerminalDisplay.h"
#include "TerminalInput.h"

class Application {
public:
	Application();
	~Application() = default;

	void run();

private:
	TerminalDisplay terminalDisplay;
	TerminalInput terminalInput;
};
