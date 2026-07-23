#include "Application.h"

#include <iostream>
#include <string_view>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace {

void printHelp() {
    std::cout
        << "FScope " << FSCOPE_VERSION << '\n'
        << "Usage: fscope [--help] [--version]\n\n"
        << "Run without options to start the interactive disk-usage "
           "viewer.\n";
}

}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc > 1) {
        const std::string_view argument = argv[1];

        if (argument == "--version" || argument == "-v") {
            std::cout << "FScope " << FSCOPE_VERSION << '\n';
            return 0;
        }

        if (argument == "--help" || argument == "-h") {
            printHelp();
            return 0;
        }

        std::cerr << "Unknown option: " << argument << "\n\n";
        printHelp();
        return 1;
    }

    Application app;
    app.run();

    return 0;

}
