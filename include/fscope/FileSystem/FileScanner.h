#pragma once
#include <string>
#include <vector>
#include "FileEntry.h"

struct ScanProgress {
    std::atomic<std::uintmax_t> bytesScanned{0};
    std::atomic<std::uint64_t> filesScanned{0};
    std::atomic<std::uint64_t> directoriesScanned{0};
    std::atomic<std::uint64_t> failedEntries{0};
    std::atomic<bool> finished{false};
};

class FileScanner {
public:
    FileScanner() = default;
    ~FileScanner() = default;

    void calculateFileSize(FileEntry* entry);

    void scanEntryRecursively(FileEntry* entry, ScanProgress& progress);

private:
};

