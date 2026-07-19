#include "PlatformShell.h"

#ifdef _WIN32
#include <Windows.h>
#include <shellapi.h>
#endif

bool openInFileManager(const std::filesystem::path& path)
{
#ifdef _WIN32
    const auto result = reinterpret_cast<INT_PTR>(
        ShellExecuteW(
            nullptr,
            L"open",
            path.c_str(),
            nullptr,
            nullptr,
            SW_SHOWNORMAL
        )
    );

    return result > 32;
#else
    // Linux
    return false;
#endif
}