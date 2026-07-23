# FScope

FScope is an interactive terminal disk-usage explorer inspired by TreeSize.
It scans Windows drives, sorts their contents by size, and lets you navigate
the resulting file tree without leaving the terminal.

## Screenshots

### Drive Selection

![FScope drive selection](docs/screenshots/drive-selection.png)

Select a drive with the arrow keys or its displayed number, then press Enter
to begin scanning.

### Directory Contents

![FScope directory contents](docs/screenshots/directory-contents.png)

Browse size-sorted scan results with each entry's type, total size, and
percentage of its parent directory.

## Technical Details

FScope enumerates available Windows drives and represents the selected drive
as an in-memory tree of `FileEntry` objects. Its recursive scanner calculates
file and directory totals, tracks file counts, and sorts every directory from
largest to smallest. Symbolic links and junctions are displayed but are not
followed, preventing recursive link loops.

The interface is rendered by FTXUI and uses an event-driven application loop.
Drive scanning runs on a background `std::jthread`, while atomic progress
counters provide live file, directory, byte, and inaccessible-entry totals.
The UI only reads the completed file tree after synchronizing with the scan
worker. Scans that encounter inaccessible entries remain navigable and are
reported as partial rather than discarding the results that were collected.

Controls:

- `Up`/`Down`, `j`/`k`, or a displayed number selects an entry.
- `Enter` scans the selected drive or opens the selected directory.
- `Backspace` edits a typed selection number.
- `b` or `Esc` returns to the parent directory.
- `o` opens the current directory in Windows File Explorer.
- `q` exits FScope and cooperatively cancels an active scan.

Build with Visual Studio 2022 Build Tools and CMake 3.20 or newer:

```powershell
cmake --preset windows-msvc
cmake --build --preset windows-debug
```

The Debug executable is written to `build/Debug/FScope.exe`. CMake downloads
the pinned FTXUI dependency automatically during the first configure.

## Tech Used

- C++20
- CMake with `FetchContent`
- FTXUI 7.0.1
- C++ Standard Library filesystem, threading, atomics, and formatting
- Windows API for drive enumeration and File Explorer integration
