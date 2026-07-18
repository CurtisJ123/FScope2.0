#include "FileScanner.h"

namespace fs = std::filesystem;

void FileScanner::calculateFileSize(FileEntry* entry){
    fs::path filePath = entry->path;
    entry->size = fs::file_size(filePath);
}

void FileScanner::scanEntryRecursively(FileEntry* entry){
    if(entry->isDirectory){
        if (fs::exists(entry->path)) {
            
            for (const auto& e : fs::directory_iterator(entry->path)) {
                std::unique_ptr<FileEntry> newFileEntry = std::make_unique<FileEntry>();
                try {
                    if(e._Is_symlink_or_junction()) newFileEntry->isDirectory = false;
                    else if(e.is_directory()) newFileEntry->isDirectory = true;
                    newFileEntry->parent = entry;
                    newFileEntry->path = e.path();
                    scanEntryRecursively(newFileEntry.get());

                } catch (const std::exception& ex) {
                    newFileEntry->scanFailed = true;
                    newFileEntry->scanError = ex.what();
                }
                entry->children.push_back(std::move(newFileEntry));
            }
            uintmax_t directorySize = 0;
            for(auto& child : entry->children){
                directorySize += child->size;
            }
            entry->size = directorySize;
        }
    }else{
        calculateFileSize(entry);
    }
}
