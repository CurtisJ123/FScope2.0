cmake_minimum_required(VERSION 3.20)

project(FScope VERSION 0.1.0 LANGUAGES CXX)

add_executable(FScope
    src/main.cpp
    src/app/Application.cpp
    src/terminal/TerminalDisplay.cpp
    src/terminal/TerminalInput.cpp
    src/FileSystem/FileSystem.cpp
    src/FileSystem/FileEntry.cpp
    src/FileSystem/FileScanner.cpp
    src/FileSystem/PlatformShell.cpp
)

target_include_directories(FScope PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/fscope/terminal"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/fscope/app"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/fscope/FileSystem"
)

if(WIN32)
    target_link_libraries(FScope PRIVATE Shell32)
endif()

target_compile_features(FScope PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(FScope PRIVATE /W4 /permissive-)
else()
    target_compile_options(FScope PRIVATE -Wall -Wextra -Wpedantic)
endif()
