#pragma once
#include <string>
#include <vector>
#include "FileEntry.h"

class FileScanner {
public:
    FileScanner() = default;
    ~FileScanner() = default;

    void calculateFileSize(FileEntry* entry);

    void scanEntryRecursively(FileEntry* entry);

private:
};

