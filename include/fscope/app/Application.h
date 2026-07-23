#pragma once

#include "TerminalDisplay.h"

#include "FileSystem.h"

class Application {
public:
	Application();
	~Application() = default;

	void run();

private:
	TerminalDisplay terminalDisplay;
	FileSystem fileSystem;
};
