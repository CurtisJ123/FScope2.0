#pragma once

#include "TerminalDisplay.h"
#include "TerminalInput.h"

#include "FileSystem.h"
#include "FileEntry.h"

#include "PlatformShell.h"

#include <charconv>
#include <thread>


class Application {
public:
	Application();
	~Application() = default;

	void run();

private:
	TerminalDisplay terminalDisplay;
	TerminalInput terminalInput;
	FileSystem fileSystem;
};
