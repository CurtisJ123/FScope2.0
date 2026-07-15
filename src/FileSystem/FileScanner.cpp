#include "FileScanner.h"


void FileScanner::setFileSize(FileEntry* entry){
    std::filesystem::path filePath = entry->path;
    entry->size = std::filesystem::file_size(filePath);
}