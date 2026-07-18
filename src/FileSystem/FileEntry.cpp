#include "FileEntry.h"

FileEntry::FileEntry(){};

std::string FileEntry::name() const{
    if (path.empty()) {
        return "No path was assigned";
    }

    if (path == path.root_path()) {
        return path.root_path().string();
    }

    return path.filename().string();
}