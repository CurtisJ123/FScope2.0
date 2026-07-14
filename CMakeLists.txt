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
)

target_include_directories(FScope PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/fscope/terminal"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/fscope/app"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/fscope/FileSystem"
)

target_compile_features(FScope PRIVATE cxx_std_20)
