#pragma once
#include <string>
#include <vector>
#include "FileEntry.h"

class FileScanner {
public:
    FileScanner() = default;
    ~FileScanner() = default;

    void setFileSize(FileEntry* entry);

    void setAllChildren(FileEntry* entry);

private:
};

