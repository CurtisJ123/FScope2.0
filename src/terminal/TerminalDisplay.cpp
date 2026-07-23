#include "TerminalDisplay.h"

#include <ftxui/dom/elements.hpp>

#include <algorithm>
#include <array>
#include <format>
#include <string_view>
#include <utility>

namespace {

std::string pathToUtf8(const std::filesystem::path& path) {
    const std::u8string utf8Path = path.u8string();
    return {
        reinterpret_cast<const char*>(utf8Path.data()),
        utf8Path.size()
    };
}

std::string entryType(const FileEntry& entry) {
    if (entry.isLink) {
        return "Link";
    }

    return entry.isDirectory ? "Directory" : "File";
}

ftxui::Element rightAligned(std::string value, int width) {
    using namespace ftxui;

    return hbox({
        filler(),
        text(std::move(value)),
    }) | size(WIDTH, EQUAL, width);
}

}

std::string TerminalDisplay::formatSize(std::uintmax_t bytes) const {
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

std::vector<std::string> TerminalDisplay::buildMenuEntries(
    const FileSystem& fileSystem,
    const ViewState& view
) const {
    std::vector<std::string> entries;

    if (view.currentState == StateType::DriveSelection) {
        entries.reserve(fileSystem.drives.size());

        for (const auto& drive : fileSystem.drives) {
            entries.push_back(drive->name());
        }

        return entries;
    }

    if (view.currentState != StateType::EntryContents ||
        view.selectedEntry == nullptr) {
        return entries;
    }

    entries.reserve(view.selectedEntry->children.size());

    for (const auto& child : view.selectedEntry->children) {
        entries.push_back(child->name());
    }

    return entries;
}

ftxui::MenuOption TerminalDisplay::buildMenuOption(
    const FileSystem& fileSystem,
    const ViewState& view
) const {
    using namespace ftxui;

    MenuOption option = MenuOption::Vertical();
    option.entries_option.transform =
        [this, &fileSystem, &view](const EntryState& state) {
            Element row;

            if (view.currentState == StateType::DriveSelection &&
                state.index >= 0 &&
                state.index < static_cast<int>(fileSystem.drives.size())) {
                row = hbox({
                    text(state.active ? "> " : "  ") |
                        color(Color::Cyan),
                    rightAligned(
                        std::format("{}.", state.index + 1),
                        5
                    ),
                    text(" "),
                    text(
                        fileSystem.drives[
                            static_cast<std::size_t>(state.index)
                        ]->name()
                    ) | flex,
                });
            } else if (
                view.currentState == StateType::EntryContents &&
                view.selectedEntry != nullptr &&
                state.index >= 0 &&
                state.index < static_cast<int>(
                    view.selectedEntry->children.size()
                )
            ) {
                const FileEntry& child =
                    *view.selectedEntry->children[
                        static_cast<std::size_t>(state.index)
                    ];
                const double percentOfParent =
                    view.selectedEntry->size == 0
                        ? 0.0
                        : static_cast<double>(child.size) /
                            static_cast<double>(view.selectedEntry->size) *
                            100.0;

                row = hbox({
                    text(state.active ? "> " : "  ") |
                        color(Color::Cyan),
                    rightAligned(
                        std::format("{}.", state.index + 1),
                        7
                    ),
                    text(" "),
                    text(child.name()) | flex,
                    text(entryType(child)) |
                        size(WIDTH, EQUAL, 12),
                    rightAligned(formatSize(child.size), 13),
                    rightAligned(
                        std::format("{:.1f}%", percentOfParent),
                        11
                    ),
                });
            } else {
                row = text(state.label);
            }

            if (state.focused) {
                row = std::move(row) | inverted;
            }
            if (state.active) {
                row = std::move(row) | bold;
            }

            return row;
        };

    return option;
}

ftxui::Element TerminalDisplay::render(
    const FileSystem& fileSystem,
    const ViewState& view,
    ftxui::Element menu
) const {
    using namespace ftxui;

    std::string location = "Drive selection";
    if (view.selectedEntry != nullptr) {
        location = pathToUtf8(view.selectedEntry->path);
    }

    Element body;
    switch (view.currentState) {
        case StateType::DriveSelection:
            body = renderDriveSelection(fileSystem, std::move(menu));
            break;
        case StateType::Scanning:
            body = renderScanning(fileSystem, view);
            break;
        case StateType::EntryContents:
            body = renderEntryContents(view, std::move(menu));
            break;
        case StateType::Error:
            body = renderError(view);
            break;
    }

    return vbox({
        hbox({
            text(" FScope ") | bold | color(Color::Cyan),
            filler(),
            text(location) | dim,
        }),
        separator(),
        std::move(body) | flex,
        separator(),
        renderStatus(view),
        text(commandHint(view.currentState)) | dim,
    }) | border;
}

ftxui::Element TerminalDisplay::renderDriveSelection(
    const FileSystem& fileSystem,
    ftxui::Element menu
) const {
    using namespace ftxui;

    if (fileSystem.drives.empty()) {
        return vbox({
            filler(),
            hbox({
                filler(),
                text("No drives were found.") | color(Color::Red),
                filler(),
            }),
            filler(),
        });
    }

    return vbox({
        text("Available drives") | bold,
        text("Choose a drive to scan.") | dim,
        separator(),
        std::move(menu) | vscroll_indicator | frame | flex,
    });
}

ftxui::Element TerminalDisplay::renderScanning(
    const FileSystem& fileSystem,
    const ViewState& view
) const {
    using namespace ftxui;

    const std::string entryName =
        view.selectedEntry == nullptr
            ? "selected entry"
            : view.selectedEntry->name();

    auto counter = [](std::string label, std::string value) {
        return hbox({
            text(std::move(label)),
            filler(),
            text(std::move(value)) | bold,
        });
    };

    return vbox({
        filler(),
        hbox({
            filler(),
            vbox({
                text(std::format("Scanning {}...", entryName)) |
                    bold | color(Color::Cyan),
                separator(),
                counter(
                    "Files scanned",
                    std::to_string(fileSystem.progress.filesScanned.load())
                ),
                counter(
                    "Directories found",
                    std::to_string(
                        fileSystem.progress.directoriesScanned.load()
                    )
                ),
                counter(
                    "Data found",
                    formatSize(fileSystem.progress.bytesScanned.load())
                ),
                counter(
                    "Inaccessible entries",
                    std::to_string(fileSystem.progress.failedEntries.load())
                ),
                separator(),
                text("Totals update as the file tree is discovered.") | dim,
            }) | border,
            filler(),
        }),
        filler(),
    });
}

ftxui::Element TerminalDisplay::renderEntryContents(
    const ViewState& view,
    ftxui::Element menu
) const {
    using namespace ftxui;

    if (view.selectedEntry == nullptr ||
        !view.selectedEntry->isDirectory) {
        return text("No directory is selected.") | color(Color::Red);
    }

    std::vector<Element> contents{
        hbox({
            text(std::format(
                "Total size: {}",
                formatSize(view.selectedEntry->size)
            )),
            text("   "),
            text(std::format(
                "Files: {}",
                view.selectedEntry->fileCount
            )),
            text("   "),
            text(std::format(
                "Scan: {}",
                scanStatus(view.scanningState)
            )),
        }),
        separator(),
        hbox({
            text("  "),
            rightAligned("#", 7),
            text(" "),
            text("Name") | flex,
            text("Type") | size(WIDTH, EQUAL, 12),
            rightAligned("Size", 13),
            rightAligned("Parent %", 11),
        }) | bold,
        separator(),
    };

    if (view.selectedEntry->children.empty()) {
        contents.push_back(
            text("No child entries were found.") | dim
        );
        contents.push_back(filler());
    } else {
        contents.push_back(
            std::move(menu) | vscroll_indicator | frame | flex
        );
    }

    return vbox(std::move(contents));
}

ftxui::Element TerminalDisplay::renderError(const ViewState& view) const {
    using namespace ftxui;

    return vbox({
        filler(),
        hbox({
            filler(),
            vbox({
                text("The scan could not be completed.") |
                    bold | color(Color::Red),
                separator(),
                text(view.statusMessage),
            }) | border,
            filler(),
        }),
        filler(),
    });
}

ftxui::Element TerminalDisplay::renderStatus(
    const ViewState& view
) const {
    using namespace ftxui;

    if (view.currentState == StateType::Error) {
        return text("");
    }

    if (!view.selectionInput.empty()) {
        std::string selectionStatus = std::format(
            "Number: {}   Enter: Choose   Backspace: Edit",
            view.selectionInput
        );

        if (!view.statusMessage.empty()) {
            selectionStatus += std::format(
                "   {}",
                view.statusMessage
            );
        }

        return text(std::move(selectionStatus)) |
            color(
                view.statusMessage.empty()
                    ? Color::Cyan
                    : Color::Yellow
            );
    }

    if (view.statusMessage.empty()) {
        return text("");
    }

    const Color statusColor =
        view.scanningState == ScanningState::Partial ||
        view.currentState == StateType::Scanning
            ? Color::Yellow
            : Color::Cyan;

    return text(view.statusMessage) | color(statusColor);
}

std::string TerminalDisplay::commandHint(StateType state) const {
    switch (state) {
        case StateType::DriveSelection:
            return " Up/Down or number: Select   Enter: Scan   q: Quit";
        case StateType::Scanning:
            return " Scanning in progress   q: Cancel scan and quit";
        case StateType::EntryContents:
            return " Up/Down or number: Select   Enter: Open directory   "
                   "b/Esc: Back   o: File Explorer   q: Quit";
        case StateType::Error:
            return " b/Esc: Back to drives   q: Quit";
    }

    return {};
}

std::string TerminalDisplay::scanStatus(ScanningState state) const {
    switch (state) {
        case ScanningState::NotStarted:
            return "Not started";
        case ScanningState::InProgress:
            return "Scanning";
        case ScanningState::Complete:
            return "Complete";
        case ScanningState::Partial:
            return "Partial";
        case ScanningState::Failed:
            return "Failed";
    }

    return "Unknown";
}
