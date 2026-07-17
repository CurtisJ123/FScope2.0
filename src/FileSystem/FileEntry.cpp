#include "FileEntry.h"

FileEntry::FileEntry(){};

std::string FileEntry::name() const{
    if(!path.empty()) return path.filename().string();
    else return "No path was assigned";
}