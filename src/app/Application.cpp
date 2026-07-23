#include "Application.h"
#include <algorithm>
#include <cctype>
#include <charconv>
#include <chrono>
#include <exception>
#include <format>
#include <thread>
#include <utility>

namespace {

std::string normalizeInput(std::string input) {
    const auto isNotWhitespace = [](unsigned char character) {
        return std::isspace(character) == 0;
    };

    input.erase(
        input.begin(),
        std::find_if(
            input.begin(),
            input.end(),
            [&](char character) {
                return isNotWhitespace(static_cast<unsigned char>(character));
            }
        )
    );

    input.erase(
        std::find_if(
            input.rbegin(),
            input.rend(),
            [&](char character) {
                return isNotWhitespace(static_cast<unsigned char>(character));
            }
        ).base(),
        input.end()
    );

    std::transform(
        input.begin(),
        input.end(),
        input.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        }
    );

    return input;
}

}

Application::Application()
    : terminalDisplay(), terminalInput(), fileSystem() {}

void Application::run(){
    fileSystem.initializeDrives();

    ViewState view;
    view.currentState = StateType::DriveSelection;
    terminalDisplay.display(fileSystem, view);

    int choice = 0;
    bool selectingDrive = true;

    while(true){
        // Read a valid menu selection.
        while(true){
            std::string input;
            if (!terminalInput.readLine(input)) {
                return;
            }

            input = normalizeInput(std::move(input));

            if (input == "q") {
                return;
            }
            else if (input == "b") {
                if (view.selectedEntry == nullptr) {
                    continue;
                }

                if(view.selectedEntry->parent != nullptr){
                    view.selectedEntry = view.selectedEntry->parent;
                    view.currentState = StateType::EntryContents;
                }else{
                    view.selectedEntry = nullptr;
                    view.currentState = StateType::DriveSelection;
                    selectingDrive = true;
                }
                view.statusMessage.clear();
                terminalDisplay.display(fileSystem, view);
                continue;
            }
            else if (input == "o") {
                if (view.selectedEntry == nullptr) {
                    view.statusMessage = "No directory is selected.";
                }
                else if (!openInFileManager(view.selectedEntry->path)) {
                    view.statusMessage = "Could not open the directory.";
                }
                else {
                    view.statusMessage = "Opened directory in File Explorer.";
                }

                terminalDisplay.display(fileSystem, view);
                continue;
            }else {
                auto [end, error] = std::from_chars(input.data(), input.data() + input.size(), choice);

                if (error != std::errc{} || end != input.data() + input.size()) {
                    choice = 0;
                }

            }

            if(selectingDrive){
                if (choice < 1 ||
                    choice > static_cast<int>(fileSystem.drives.size())) {
                    view.statusMessage = "Invalid selection.";
                    view.currentState = StateType::DriveSelection;
                    terminalDisplay.display(fileSystem, view);
                    continue;
                }
            }else{
                if (view.selectedEntry == nullptr ||
                    choice < 1 ||
                    choice > static_cast<int>(view.selectedEntry->children.size())) {
                    view.statusMessage = "Invalid selection.";
                    view.currentState = StateType::EntryContents;
                    terminalDisplay.display(fileSystem, view);
                    continue;
                }

                const auto& selectedChild =
                    view.selectedEntry->children.at(static_cast<std::size_t>(choice - 1));

                if (!selectedChild->isDirectory) {
                    view.statusMessage = "Please select a directory.";
                    view.currentState = StateType::EntryContents;
                    terminalDisplay.display(fileSystem, view);
                    continue;
                }
            }
            break;
        }

        view.selectedIndex = choice - 1;
        view.statusMessage.clear();

        if(selectingDrive){
            view.selectedEntry =
                fileSystem.drives.at(static_cast<std::size_t>(view.selectedIndex)).get();
        }else{
            view.selectedEntry =
                view.selectedEntry->children.at(
                    static_cast<std::size_t>(view.selectedIndex)).get();
        }

        if(selectingDrive) {
            view.currentState = StateType::Scanning;
            view.scanningState = ScanningState::InProgress;
            fileSystem.progress.reset();
            terminalDisplay.display(fileSystem, view);

            try {
                std::exception_ptr scanError;
                FileEntry* const entryToScan = view.selectedEntry;

                std::jthread worker([this, entryToScan, &scanError] {
                    try {
                        fileSystem.scanEntry(entryToScan);
                    } catch (...) {
                        scanError = std::current_exception();
                    }

                    fileSystem.progress.finished.store(true);
                });

                while (!fileSystem.progress.finished.load()) {
                    terminalDisplay.display(fileSystem, view);
                    std::this_thread::sleep_for(std::chrono::milliseconds(250));
                }

                worker.join();

                if (scanError != nullptr) {
                    std::rethrow_exception(scanError);
                }

                const std::uint64_t failedEntries =
                    fileSystem.progress.failedEntries.load();

                if (entryToScan->scanFailed &&
                    entryToScan->children.empty()) {
                    view.scanningState = ScanningState::Failed;
                    view.currentState = StateType::Error;
                    view.statusMessage =
                        entryToScan->scanError.empty()
                            ? "The selected drive could not be scanned."
                            : entryToScan->scanError;
                } else if (failedEntries > 0) {
                    view.scanningState = ScanningState::Partial;
                    view.statusMessage = std::format(
                        "{} {} could not be fully scanned.",
                        failedEntries,
                        failedEntries == 1 ? "entry" : "entries"
                    );
                    view.currentState = StateType::EntryContents;
                } else {
                    view.scanningState = ScanningState::Complete;
                    view.currentState = StateType::EntryContents;
                }
            }
            catch (const std::filesystem::filesystem_error& error) {
                view.scanningState = ScanningState::Failed;
                view.currentState = StateType::Error;
                view.statusMessage = error.what();
            }
            catch (const std::exception& error) {
                view.scanningState = ScanningState::Failed;
                view.currentState = StateType::Error;
                view.statusMessage = error.what();
            }
            catch (...) {
                view.scanningState = ScanningState::Failed;
                view.currentState = StateType::Error;
                view.statusMessage = "An unknown scan error occurred.";
            }
        } else {
            view.currentState = StateType::EntryContents;
        }

        terminalDisplay.display(fileSystem, view);
        selectingDrive = false;
    }
}
