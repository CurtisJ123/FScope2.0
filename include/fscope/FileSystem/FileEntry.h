#pragma once
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <filesystem>


class FileEntry {
public:
	std::filesystem::path path;
	
	std::vector<std::unique_ptr<FileEntry>> children;
	FileEntry* parent = nullptr;
	std::uintmax_t size = 0;
	bool isDirectory = false;
	bool scanFailed = false;
	std::string scanError;

	
	FileEntry();
	explicit FileEntry(std::filesystem::path entryPath) : path(std::move(entryPath)){}
	~FileEntry() = default;

	std::string name() const;

};
