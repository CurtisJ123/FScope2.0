# FScope

FScope is a terminal-based disk usage viewer inspired by TreeSize. It scans a
selected Windows drive, calculates directory sizes, and lets you navigate the
resulting file tree.

## Current features

- Lists available Windows drives.
- Recursively scans files and directories.
- Shows live file, directory, byte, and failure counts while scanning.
- Sorts directory contents from largest to smallest.
- Displays human-readable sizes and each entry's percentage of its parent.
- Navigates into directories and back to their parent.
- Opens the current directory in Windows File Explorer.
- Reports partial scans when entries cannot be accessed.

## Build

Requirements:

- Windows 10 or newer
- Visual Studio 2022 Build Tools with the C++ workload
- CMake 3.20 or newer

```powershell
cmake --preset windows-msvc
cmake --build --preset windows-debug
```

The debug executable is created at:

```text
build/Debug/FScope.exe
```

## Controls

- Enter a displayed number to select a drive or directory.
- `b` returns to the parent directory.
- `o` opens the current directory in File Explorer.
- `q` exits FScope.

Commands are case-insensitive and may contain surrounding whitespace.

## Known limitations

- FScope currently supports Windows only.
- A scan cannot yet be cancelled once it has started.
- Junctions and symbolic links are shown as zero-byte link entries and are not
  followed.
- Inaccessible entries are excluded from calculated totals and produce a
  partial scan result.
- Displayed sizes are logical file sizes, not allocated size on disk.
