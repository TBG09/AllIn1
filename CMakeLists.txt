cmake_minimum_required(VERSION 4.0)
project(AllIn1)

set(CMAKE_CXX_STANDARD 20)

# --- External Dependencies ---
include(FetchContent)
FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse
  GIT_TAG v2.9
)
FetchContent_MakeAvailable(argparse)

# --- Common Library ---
file(GLOB_RECURSE COMMON_SOURCES "src/common/*.cpp")
add_library(common STATIC ${COMMON_SOURCES})
target_include_directories(common PUBLIC include)

# --- IO Library ---
file(GLOB_RECURSE IO_SOURCES "src/io/*.cpp")
add_library(io STATIC ${IO_SOURCES})
target_include_directories(io PUBLIC include)
target_link_libraries(io PRIVATE common)

# --- Main Executable ---
add_executable(AllIn1 src/main.cpp)
target_link_libraries(AllIn1 PRIVATE argparse io)
target_include_directories(AllIn1 PUBLIC include)
