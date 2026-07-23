#pragma once
#include <atomic>
#include <cstdint>
#include <stop_token>
#include <string>
#include <vector>
#include "FileEntry.h"

struct ScanProgress {
    std::atomic<std::uintmax_t> bytesScanned{0};
    std::atomic<std::uint64_t> filesScanned{0};
    std::atomic<std::uint64_t> directoriesScanned{0};
    std::atomic<std::uint64_t> failedEntries{0};
    std::atomic<bool> finished{false};

    void reset() noexcept {
        bytesScanned.store(0);
        filesScanned.store(0);
        directoriesScanned.store(0);
        failedEntries.store(0);
        finished.store(false);
    }
};

class FileScanner {
public:
    FileScanner() = default;
    ~FileScanner() = default;

    void calculateFileSize(FileEntry* entry);

    void scanEntryRecursively(
        FileEntry* entry,
        ScanProgress& progress,
        std::stop_token stopToken = {}
    );

private:
};

