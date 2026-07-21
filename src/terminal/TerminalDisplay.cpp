#include "TerminalDisplay.h"

TerminalDisplay::TerminalDisplay()
    : displayText() {}

void TerminalDisplay::display() const {
    std::cout << displayText;
}

void TerminalDisplay::setBufferText(const std::string& dt) {
    displayText = dt;
}

void TerminalDisplay::setDisplayText() {
    displayText = "";
}

void TerminalDisplay::clearBuffer(){
    displayText = "";
}

void TerminalDisplay::appendText(const std::string& at){
    displayText += at;
}

void TerminalDisplay::appendText(const int& number){
    displayText += std::to_string(number);
}

std::string TerminalDisplay::formatSize(std::uintmax_t bytes) const
{
    constexpr std::array<std::string_view, 5> units{
        "B", "KB", "MB", "GB", "TB"
    };

    double value = static_cast<double>(bytes);
    std::size_t unitIndex = 0;

    while (value >= 1024.0 &&
           unitIndex + 1 < units.size()) {
        value /= 1024.0;
        ++unitIndex;
    }

    if (unitIndex == 0) {
        return std::format(
            "{} {}",
            bytes,
            units[unitIndex]
        );
    }

    return std::format(
        "{:.1f} {}",
        value,
        units[unitIndex]
    );
}

void TerminalDisplay::display(const FileSystem& fileSystem, const ViewState& view){
    switch(view.currentState){
        case StateType::DriveSelection:
            clearBuffer();
            appendText("\n\nFScope\n\n");
            appendText("Available Drives:\n\n");
            if (fileSystem.drives.empty()) {
                appendText("No drives were found.\n");
                break;
            }

            for(std::size_t i = 0; i < fileSystem.drives.size(); ++i){
                appendText(std::format("{}. {}\n", i + 1, fileSystem.drives[i]->name()));
            }

            if (!view.statusMessage.empty()) {
                appendText(std::format("\n{}\n", view.statusMessage));
            }

            appendText("\nCommands: q = Quit\n");
            appendText(std::format("Select a drive [1-{}]: ", fileSystem.drives.size()));
            break;

        case StateType::Scanning:
            clearBuffer();
            appendText("\n\nFScope\n\n");

            if (view.selectedEntry == nullptr) {
                appendText("Scanning...");
            } else {
                appendText(std::format("Scanning {}...\n", view.selectedEntry->name()));
                appendText(std::format("File Count: {}...\n", fileSystem.progress.filesScanned.load()));
                appendText(std::format("Size {}...\n", formatSize(fileSystem.progress.bytesScanned.load())));
                appendText(std::format("Failed Entries {}...\n", fileSystem.progress.failedEntries.load()));
            }
            break;

        case StateType::EntryContents: {
            clearBuffer();

            if (view.selectedEntry == nullptr || !view.selectedEntry->isDirectory) {
                appendText("No directory is selected.\n");
                break;
            }

            appendText(std::format("\n\nFScope - {}\n\n", view.selectedEntry->name()));

            appendText(std::format("Total size: {}\n", formatSize(view.selectedEntry->size)));

            std::string scanStatus;

            switch (view.scanningState) {
                case ScanningState::NotStarted:
                    scanStatus = "Not started";
                    break;

                case ScanningState::InProgress:
                    scanStatus = "Scanning";
                    break;

                case ScanningState::Complete:
                    scanStatus = "Complete";
                    break;

                case ScanningState::Partial:
                    scanStatus = "Partial";
                    break;

                case ScanningState::Failed:
                    scanStatus = "Failed";
                    break;
            }

            appendText(std::format("Scan status: {}\n", scanStatus));

            if (!view.statusMessage.empty()) {
                appendText(std::format("\n{}\n", view.statusMessage));
            }

            appendText("\n");

            appendText(std::format(
                "{:>4}  {:<30} {:<12} {:>12} {:>9}\n",
                "#",
                "Name",
                "Type",
                "Size",
                "Parent %"
            ));

            appendText(std::string(72, '-') + "\n");

            for (std::size_t i = 0; i < view.selectedEntry->children.size(); ++i) {
                const auto& child = view.selectedEntry->children[i];
                const std::string entryType = child->isDirectory ? "Directory" : "File";
                const double percentOfParent =
                    view.selectedEntry->size == 0
                        ? 0.0
                        : static_cast<double>(child->size) /
                            static_cast<double>(view.selectedEntry->size) * 100.0;

                appendText(std::format(
                    "{:>4}. {:<30} {:<12} {:>12} {:>8.1f}%\n",
                    i + 1,
                    child->name(),
                    entryType,
                    formatSize(child->size),
                    percentOfParent
                ));
            }

            if (view.selectedEntry->children.empty()) {
                appendText("\nNo child entries were found.\n");
            }

            appendText("\nCommands: o = Open, b = Back, q = Quit\n");

            if (view.selectedEntry->children.empty()) {
                appendText("Enter a command: ");
            } else {
                appendText(std::format(
                    "Select a directory [1-{}]: ",
                    view.selectedEntry->children.size()
                ));
            }
            break;
        }
        case StateType::Error:
            clearBuffer();
            appendText("\n\nFScope\n\n");
            appendText(view.statusMessage);
            appendText("\n\nCommands: b = Back, q = Quit\n");
            appendText("Enter a command: ");
            break;
    }
    std::cout << displayText << std::flush;
}
