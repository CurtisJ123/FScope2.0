#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <utility>
#include <FileEntry.h>
#include <windows.h>


class FileSystem {
public:
	FileSystem();
	~FileSystem() = default;

	std::vector<std::unique_ptr<FileEntry>> drives;

	void initializeDrives();
};
