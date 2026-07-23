#pragma once
#include <cstdint>
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
	bool isLink = false;
	bool scanFailed = false;
	std::string scanError;
	std::uint64_t fileCount = 0;

	
	FileEntry();
	explicit FileEntry(std::filesystem::path entryPath) : path(std::move(entryPath)){}
	~FileEntry() = default;

	std::string name() const;

};
