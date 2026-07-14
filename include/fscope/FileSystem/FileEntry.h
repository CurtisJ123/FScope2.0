#pragma once
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <filesystem>


class FileEntry {
public:
	std::filesystem::path path;
	std::string name;
	std::vector<std::unique_ptr<FileEntry>> children;
	FileEntry* parent = nullptr;
	std::uintmax_t size = 0;
	bool isDirectory = false;

	
	FileEntry();
	explicit FileEntry(std::filesystem::path entryPath) : path(std::move(entryPath)){}
	~FileEntry() = default;

	void getChildren();

	void calculateSize();

};
