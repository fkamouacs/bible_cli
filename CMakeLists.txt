cmake_minimum_required(VERSION 3.14)
project(bible_viewer)

set(CMAKE_CXX_STANDARD 17)

# Find packages
find_package(Curses REQUIRED)
find_package(SQLite3 REQUIRED)

# Include directories
include_directories(${CURSES_INCLUDE_DIR})
include_directories(${SQLITE3_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    src/database.cpp
    src/utils.cpp
)

# Add executable
add_executable(bible_viewer ${SOURCES})

# Link libraries
target_link_libraries(bible_viewer ${CURSES_LIBRARIES} ${SQLITE3_LIBRARIES})
target_link_libraries(bible_viewer menu ncurses sqlite3)
