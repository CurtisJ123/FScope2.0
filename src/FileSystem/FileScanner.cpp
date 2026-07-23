#include "FileScanner.h"
#include <algorithm>

namespace fs = std::filesystem;

void FileScanner::calculateFileSize(FileEntry* entry){
    entry->size = fs::file_size(entry->path);
}

void FileScanner::scanEntryRecursively(FileEntry* entry, ScanProgress& progress){
    entry->children.clear();
    entry->size = 0;
    entry->fileCount = 0;
    entry->scanFailed = false;
    entry->scanError.clear();

    if (!entry->isDirectory) {
        calculateFileSize(entry);
        entry->fileCount = 1;
        progress.bytesScanned.fetch_add(entry->size);
        progress.filesScanned.fetch_add(1);
        return;
    }

    progress.directoriesScanned.fetch_add(1);

    try {
        if (!fs::exists(entry->path)) {
            entry->scanFailed = true;
            entry->scanError = "Path no longer exists.";
            progress.failedEntries.fetch_add(1);
            return;
        }

        for (const auto& directoryEntry : fs::directory_iterator(entry->path)) {
            auto child = std::make_unique<FileEntry>(directoryEntry.path());
            child->parent = entry;

            try {
                if (directoryEntry._Is_symlink_or_junction()) {
                    child->isLink = true;
                    child->isDirectory = false;
                } else {
                    child->isDirectory = directoryEntry.is_directory();
                    scanEntryRecursively(child.get(), progress);
                }
            } catch (const std::exception& error) {
                child->scanFailed = true;
                child->scanError = error.what();
                progress.failedEntries.fetch_add(1);
            }

            entry->children.push_back(std::move(child));
        }
    } catch (const std::exception& error) {
        entry->scanFailed = true;
        entry->scanError = error.what();
        progress.failedEntries.fetch_add(1);
    }

    for (const auto& child : entry->children) {
        entry->size += child->size;
        entry->fileCount += child->fileCount;
    }

    std::sort(
        entry->children.begin(),
        entry->children.end(),
        [](const std::unique_ptr<FileEntry>& left,
           const std::unique_ptr<FileEntry>& right) {
            if (left->size != right->size) {
                return left->size > right->size;
            }

            return left->name() < right->name();
        }
    );
}
