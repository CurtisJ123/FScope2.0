#pragma once
#include <string>
#include <vector>
#include "FileEntry.h"

class FileScanner {
public:
    FileScanner() = default;
    ~FileScanner() = default;

    void setFileSize(FileEntry* entry);

private:
};

