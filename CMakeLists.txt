cmake_minimum_required(VERSION 3.15)
project(Chronos CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compile options
if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Main library
add_library(chronos_lib
    src/Date.cpp
    src/Task.cpp
    src/Project.cpp
    src/UrgencyCalculator.cpp
    src/TaskFilter.cpp
    src/UndoManager.cpp
    src/FileHandler.cpp
    src/ProjectManager.cpp
    src/ConsoleColor.cpp
    src/OuterMenu.cpp
    src/Menu.cpp
)
target_include_directories(chronos_lib PUBLIC include)

# Main executable
add_executable(chronos src/main.cpp)
target_link_libraries(chronos PRIVATE chronos_lib)

# Tests
enable_testing()
add_subdirectory(tests)
