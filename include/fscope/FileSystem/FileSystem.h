#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <utility>
#include <FileEntry.h>
#include <windows.h>
#include <FileScanner.h>


class FileSystem {
public:
	FileSystem();
	~FileSystem() = default;

	FileScanner fileScanner;  
	std::vector<std::unique_ptr<FileEntry>> drives;

	void initializeDrives();
	std::uintmax_t getFileSize(FileEntry* entry);
};
