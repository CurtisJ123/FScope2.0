#include "FileScanner.h"

namespace fs = std::filesystem;

void FileScanner::setFileSize(FileEntry* entry){
    fs::path filePath = entry->path;
    entry->size = fs::file_size(filePath);
}

void FileScanner::setAllChildren(FileEntry* entry){
    if(entry->isDirectory){
        if (fs::exists(entry->path)) {
            
            for (const auto& e : fs::directory_iterator(entry->path)) {
                try {
                
                    std::unique_ptr<FileEntry> newFileEntry = std::make_unique<FileEntry>();
                    
                    if(e._Is_symlink_or_junction()) newFileEntry->isDirectory = false;
                    else if(e.is_directory()) newFileEntry->isDirectory = true;

                    newFileEntry->parent = entry;
                    newFileEntry->path = e.path();
                    setAllChildren(newFileEntry.get());
                    entry->children.push_back(std::move(newFileEntry));

                } catch (const std::exception& ex) {
                    // do something
                }
            }
            uintmax_t directorySize = 0;
            for(auto& child : entry->children){
                directorySize += child->size;
            }
            entry->size = directorySize;
        }
    }else{
        setFileSize(entry);
    }
}