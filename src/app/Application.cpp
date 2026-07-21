#include "Application.h"

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
            std::string input = terminalInput.readLine();

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
            terminalDisplay.display(fileSystem, view);

            try {
                std::jthread worker([&] {
                    fileSystem.scanEntry(view.selectedEntry);
                });
                
                std::uint64_t lastFileCount = 0;
                auto lastUpdate = std::chrono::steady_clock::now();
                while(fileSystem.progress.finished == false){
                    auto currentCount = fileSystem.progress.filesScanned.load();
                    auto now = std::chrono::steady_clock::now();
                    if (currentCount != lastFileCount ||
                        now - lastUpdate >= std::chrono::seconds(1)) {
                        terminalDisplay.display(fileSystem, view);
                        lastFileCount = currentCount;
                        lastUpdate = now;
                    }
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                view.scanningState = ScanningState::Complete;
                view.currentState = StateType::EntryContents;
            }
            catch (const std::filesystem::filesystem_error& error) {
                view.scanningState = ScanningState::Failed;
                view.currentState = StateType::Error;
                view.statusMessage = error.what();
            }
        } else {
            view.currentState = StateType::EntryContents;
        }

        terminalDisplay.display(fileSystem, view);
        selectingDrive = false;
    }
}
