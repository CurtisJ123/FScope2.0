#include "FileEntry.h"

namespace {

std::string pathToUtf8(const std::filesystem::path& path) {
    const std::u8string utf8Path = path.u8string();
    return {
        reinterpret_cast<const char*>(utf8Path.data()),
        utf8Path.size()
    };
}

}

FileEntry::FileEntry() = default;

std::string FileEntry::name() const{
    if (path.empty()) {
        return "No path was assigned";
    }

    if (path == path.root_path()) {
        return pathToUtf8(path.root_path());
    }

    return pathToUtf8(path.filename());
}
