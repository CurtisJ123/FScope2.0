#pragma once

#include "FileSystem.h"
#include "ViewState.h"

#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>

#include <cstdint>
#include <string>
#include <vector>

class TerminalDisplay {
public:
    TerminalDisplay() = default;

    std::vector<std::string> buildMenuEntries(
        const FileSystem& fileSystem,
        const ViewState& view
    ) const;

    ftxui::MenuOption buildMenuOption(
        const FileSystem& fileSystem,
        const ViewState& view
    ) const;

    ftxui::Element render(
        const FileSystem& fileSystem,
        const ViewState& view,
        ftxui::Element menu
    ) const;

    std::string formatSize(std::uintmax_t bytes) const;

private:
    ftxui::Element renderDriveSelection(
        const FileSystem& fileSystem,
        ftxui::Element menu
    ) const;
    ftxui::Element renderScanning(
        const FileSystem& fileSystem,
        const ViewState& view
    ) const;
    ftxui::Element renderEntryContents(
        const ViewState& view,
        ftxui::Element menu
    ) const;
    ftxui::Element renderError(const ViewState& view) const;
    ftxui::Element renderStatus(const ViewState& view) const;
    std::string commandHint(StateType state) const;
    std::string scanStatus(ScanningState state) const;
};
