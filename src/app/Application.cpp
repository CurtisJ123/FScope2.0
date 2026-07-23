#include "Application.h"

#include "PlatformShell.h"

#include <ftxui/component/app.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include <algorithm>
#include <atomic>
#include <charconv>
#include <chrono>
#include <exception>
#include <format>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

namespace {

bool isCharacter(const ftxui::Event& event, char lower, char upper) {
    return event == ftxui::Event::Character(lower) ||
           event == ftxui::Event::Character(upper);
}

std::optional<char> eventDigit(const ftxui::Event& event) {
    for (char digit = '0'; digit <= '9'; ++digit) {
        if (event == ftxui::Event::Character(digit)) {
            return digit;
        }
    }

    return std::nullopt;
}

bool isMenuNavigationEvent(const ftxui::Event& event) {
    return event == ftxui::Event::ArrowUp ||
           event == ftxui::Event::ArrowDown ||
           event == ftxui::Event::Home ||
           event == ftxui::Event::End ||
           event == ftxui::Event::PageUp ||
           event == ftxui::Event::PageDown ||
           event == ftxui::Event::Tab ||
           event == ftxui::Event::TabReverse ||
           event == ftxui::Event::Character('j') ||
           event == ftxui::Event::Character('k');
}

int childIndex(const FileEntry& parent, const FileEntry* child) {
    const auto position = std::find_if(
        parent.children.begin(),
        parent.children.end(),
        [child](const std::unique_ptr<FileEntry>& candidate) {
            return candidate.get() == child;
        }
    );

    if (position == parent.children.end()) {
        return 0;
    }

    return static_cast<int>(
        std::distance(parent.children.begin(), position)
    );
}

std::string exceptionMessage(const std::exception_ptr& error) {
    if (error == nullptr) {
        return {};
    }

    try {
        std::rethrow_exception(error);
    } catch (const std::exception& exception) {
        return exception.what();
    } catch (...) {
        return "An unknown scan error occurred.";
    }
}

}

Application::Application()
    : terminalDisplay(), fileSystem() {}

void Application::run() {
    using namespace std::chrono_literals;

    fileSystem.initializeDrives();

    ViewState view;
    view.currentState = StateType::DriveSelection;

    std::vector<std::string> menuEntries =
        terminalDisplay.buildMenuEntries(fileSystem, view);

    auto app = ftxui::App::Fullscreen();
    std::jthread scanWorker;
    std::jthread refreshWorker;
    std::exception_ptr scanError;

    const auto rebuildMenu = [&] {
        menuEntries = terminalDisplay.buildMenuEntries(fileSystem, view);

        if (menuEntries.empty()) {
            view.selectedIndex = 0;
            return;
        }

        view.selectedIndex = std::clamp(
            view.selectedIndex,
            0,
            static_cast<int>(menuEntries.size()) - 1
        );
    };

    const auto selectionCount = [&]() -> std::size_t {
        if (view.currentState == StateType::DriveSelection) {
            return fileSystem.drives.size();
        }

        if (view.currentState == StateType::EntryContents &&
            view.selectedEntry != nullptr) {
            return view.selectedEntry->children.size();
        }

        return 0;
    };

    const auto applyTypedSelection = [&] {
        std::size_t choice = 0;
        const char* const first = view.selectionInput.data();
        const char* const last = first + view.selectionInput.size();
        const auto [end, error] = std::from_chars(first, last, choice);
        const std::size_t entryCount = selectionCount();

        if (error != std::errc{} ||
            end != last ||
            choice < 1 ||
            choice > entryCount) {
            view.statusMessage = std::format(
                "There is no item numbered {}.",
                view.selectionInput
            );
            return false;
        }

        view.selectedIndex = static_cast<int>(choice - 1);
        view.statusMessage.clear();
        return true;
    };

    const auto finishScan = [&] {
        if (view.currentState != StateType::Scanning ||
            !fileSystem.progress.finished.load(std::memory_order_acquire)) {
            return;
        }

        if (scanWorker.joinable()) {
            scanWorker.join();
        }

        if (refreshWorker.joinable()) {
            refreshWorker.request_stop();
            refreshWorker.join();
        }

        const std::string errorMessage = exceptionMessage(scanError);
        if (!errorMessage.empty()) {
            view.scanningState = ScanningState::Failed;
            view.currentState = StateType::Error;
            view.statusMessage = errorMessage;
        } else if (
            view.selectedEntry != nullptr &&
            view.selectedEntry->scanFailed &&
            view.selectedEntry->children.empty()
        ) {
            view.scanningState = ScanningState::Failed;
            view.currentState = StateType::Error;
            view.statusMessage =
                view.selectedEntry->scanError.empty()
                    ? "The selected drive could not be scanned."
                    : view.selectedEntry->scanError;
        } else {
            const std::uint64_t failedEntries =
                fileSystem.progress.failedEntries.load();

            if (failedEntries > 0) {
                view.scanningState = ScanningState::Partial;
                view.statusMessage = std::format(
                    "{} {} could not be fully scanned.",
                    failedEntries,
                    failedEntries == 1 ? "entry" : "entries"
                );
            } else {
                view.scanningState = ScanningState::Complete;
                view.statusMessage.clear();
            }

            view.currentState = StateType::EntryContents;
        }

        view.selectedIndex = 0;
        view.selectionInput.clear();
        rebuildMenu();
    };

    const auto startScan = [&](FileEntry* entry) {
        view.selectedEntry = entry;
        view.selectedIndex = 0;
        view.selectionInput.clear();
        view.statusMessage.clear();
        view.scanningState = ScanningState::InProgress;
        view.currentState = StateType::Scanning;
        menuEntries.clear();

        fileSystem.progress.reset();
        scanError = nullptr;

        scanWorker = std::jthread(
            [this, entry, &scanError, &app](std::stop_token stopToken) {
                try {
                    fileSystem.scanEntry(entry, stopToken);
                } catch (...) {
                    scanError = std::current_exception();
                }

                fileSystem.progress.finished.store(
                    true,
                    std::memory_order_release
                );
                app.PostEvent(ftxui::Event::Custom);
            }
        );

        refreshWorker = std::jthread(
            [this, &app](std::stop_token stopToken) {
                while (
                    !stopToken.stop_requested() &&
                    !fileSystem.progress.finished.load(
                        std::memory_order_acquire
                    )
                ) {
                    std::this_thread::sleep_for(150ms);

                    if (!stopToken.stop_requested()) {
                        app.PostEvent(ftxui::Event::Custom);
                    }
                }
            }
        );
    };

    auto menu = ftxui::Menu(
        &menuEntries,
        &view.selectedIndex,
        terminalDisplay.buildMenuOption(fileSystem, view)
    );
    auto renderer = ftxui::Renderer(menu, [&] {
        ftxui::Element renderedMenu = ftxui::text("");
        const bool isMenuState =
            view.currentState == StateType::DriveSelection ||
            view.currentState == StateType::EntryContents;

        if (isMenuState && !menuEntries.empty()) {
            renderedMenu = menu->Render();
        }

        return terminalDisplay.render(
            fileSystem,
            view,
            std::move(renderedMenu)
        );
    });

    auto root = ftxui::CatchEvent(
        renderer,
        [&](const ftxui::Event& event) {
            if (event == ftxui::Event::Custom) {
                finishScan();
                return true;
            }

            if (isCharacter(event, 'q', 'Q')) {
                if (scanWorker.joinable()) {
                    scanWorker.request_stop();
                }
                if (refreshWorker.joinable()) {
                    refreshWorker.request_stop();
                }

                app.Exit();
                return true;
            }

            if (view.currentState == StateType::Scanning) {
                if (isCharacter(event, 'b', 'B') ||
                    event == ftxui::Event::Escape) {
                    view.statusMessage =
                        "Wait for the scan to finish, or press q to cancel "
                        "the scan and quit.";
                }

                return true;
            }

            const std::optional<char> digit = eventDigit(event);
            if (digit.has_value() &&
                (view.currentState == StateType::DriveSelection ||
                 view.currentState == StateType::EntryContents)) {
                if (view.selectionInput.size() < 9) {
                    view.selectionInput.push_back(*digit);
                    applyTypedSelection();
                } else {
                    view.statusMessage =
                        "The typed selection number is too long.";
                }

                return true;
            }

            if (event == ftxui::Event::Backspace &&
                !view.selectionInput.empty()) {
                view.selectionInput.pop_back();

                if (view.selectionInput.empty()) {
                    view.statusMessage.clear();
                } else {
                    applyTypedSelection();
                }

                return true;
            }

            if (isCharacter(event, 'b', 'B') ||
                event == ftxui::Event::Escape) {
                view.selectionInput.clear();

                if (view.selectedEntry == nullptr ||
                    view.selectedEntry->parent == nullptr) {
                    view.selectedEntry = nullptr;
                    view.selectedIndex = 0;
                    view.statusMessage.clear();
                    view.scanningState = ScanningState::NotStarted;
                    view.currentState = StateType::DriveSelection;
                    rebuildMenu();
                    return true;
                }

                FileEntry* const previousEntry = view.selectedEntry;
                FileEntry* const parent = previousEntry->parent;

                view.selectedEntry = parent;
                view.selectedIndex = childIndex(*parent, previousEntry);
                view.statusMessage.clear();
                view.currentState = StateType::EntryContents;
                rebuildMenu();
                return true;
            }

            if (view.currentState == StateType::Error) {
                return true;
            }

            if (isCharacter(event, 'o', 'O')) {
                view.selectionInput.clear();

                if (view.currentState != StateType::EntryContents ||
                    view.selectedEntry == nullptr) {
                    view.statusMessage =
                        "Select and scan a drive before opening a directory.";
                } else if (!openInFileManager(view.selectedEntry->path)) {
                    view.statusMessage =
                        "Could not open the directory in File Explorer.";
                } else {
                    view.statusMessage =
                        "Opened the current directory in File Explorer.";
                }

                return true;
            }

            if (event != ftxui::Event::Return) {
                if (isMenuNavigationEvent(event) || event.is_mouse()) {
                    if (!view.selectionInput.empty()) {
                        view.selectionInput.clear();
                        view.statusMessage.clear();
                    }

                    return false;
                }

                if (menuEntries.empty()) {
                    return true;
                }

                return false;
            }

            if (!view.selectionInput.empty()) {
                if (!applyTypedSelection()) {
                    return true;
                }

                view.selectionInput.clear();
            }

            if (view.currentState == StateType::DriveSelection) {
                if (fileSystem.drives.empty() ||
                    view.selectedIndex < 0 ||
                    view.selectedIndex >=
                        static_cast<int>(fileSystem.drives.size())) {
                    view.statusMessage = "No drive is available to scan.";
                    return true;
                }

                startScan(
                    fileSystem.drives[
                        static_cast<std::size_t>(view.selectedIndex)
                    ].get()
                );
                return true;
            }

            if (view.currentState != StateType::EntryContents ||
                view.selectedEntry == nullptr ||
                view.selectedIndex < 0 ||
                view.selectedIndex >= static_cast<int>(
                    view.selectedEntry->children.size()
                )) {
                view.statusMessage = "No entry is selected.";
                return true;
            }

            FileEntry* const selectedChild =
                view.selectedEntry->children[
                    static_cast<std::size_t>(view.selectedIndex)
                ].get();

            if (!selectedChild->isDirectory) {
                view.statusMessage =
                    selectedChild->isLink
                        ? "Links are shown but are not followed."
                        : "Files are view-only; choose a directory to "
                          "navigate into.";
                return true;
            }

            view.selectedEntry = selectedChild;
            view.selectedIndex = 0;
            view.selectionInput.clear();
            view.statusMessage.clear();
            rebuildMenu();
            return true;
        }
    );

    app.Loop(root);

    if (scanWorker.joinable()) {
        scanWorker.request_stop();
        scanWorker.join();
    }

    if (refreshWorker.joinable()) {
        refreshWorker.request_stop();
        refreshWorker.join();
    }
}
