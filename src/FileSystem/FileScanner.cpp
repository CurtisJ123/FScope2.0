#include "FileScanner.h"

namespace fs = std::filesystem;

void FileScanner::calculateFileSize(FileEntry* entry){
    fs::path filePath = entry->path;
    entry->size = fs::file_size(filePath);
}

void FileScanner::scanEntryRecursively(FileEntry* entry, ScanProgress& progress){
    entry->children.clear();
    if(entry->isDirectory){
        if (fs::exists(entry->path)) {
            try{
                for (const auto& e : fs::directory_iterator(entry->path)) {
                    std::unique_ptr<FileEntry> newFileEntry = std::make_unique<FileEntry>();
                    try {
                        if(e._Is_symlink_or_junction()) newFileEntry->isDirectory = false;
                        else if(e.is_directory()) newFileEntry->isDirectory = true;
                        newFileEntry->parent = entry;
                        newFileEntry->path = e.path();
                        progress.directoriesScanned += 1;
                        scanEntryRecursively(newFileEntry.get(), progress);

                    } catch (const std::exception& ex) {
                        newFileEntry->scanFailed = true;
                        newFileEntry->scanError = ex.what();
                    }
                    entry->children.push_back(std::move(newFileEntry));
                }
            }
            catch(const std::exception& e){
                progress.failedEntries += 1;
            }
            
            uintmax_t directorySize = 0;
            for(auto& child : entry->children){
                directorySize += child->size;
            }
            entry->size = directorySize;
            
            std::sort(entry->children.begin(), entry->children.end(), 
                [](const std::unique_ptr<FileEntry>& a, const std::unique_ptr<FileEntry>& b) {
                    if (a->size != b->size) {
                        return a->size > b->size;
                    }

                    return a->name() < b->name();
                });
        }
    }else{
        calculateFileSize(entry);
        progress.bytesScanned += entry->size;
        progress.filesScanned += 1;
    }
}
