#include "FileSystem.h"

FileSystem::FileSystem(){};

void FileSystem::initializeDrives()
{
    DWORD bufferLength = GetLogicalDriveStringsW(0, nullptr);
    if (bufferLength == 0) {
        std::cerr << "Failed to get drive strings buffer size.\n";
        return;
    }

    std::vector<wchar_t> buffer(bufferLength);

    if (GetLogicalDriveStringsW(bufferLength, buffer.data()) == 0) {
        std::cerr << "Failed to retrieve drive strings.\n";
        return;
    }

    drives.clear();

    const wchar_t* drive = buffer.data();

    while (*drive != L'\0') {
        // Logical drive paths are normally ASCII values such as L"C:\\".
        std::wstring widePath(drive);
        std::string drivePathName(widePath.begin(), widePath.end());

        std::filesystem::path drivePath{drive};

        auto entry = std::make_unique<FileEntry>(drivePath);
        entry->name = drivePathName;
        entry->isDirectory = true;
        entry->parent = nullptr;

        drives.push_back(std::move(entry));

        drive += widePath.length() + 1;
    }
}
