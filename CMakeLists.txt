cmake_minimum_required(VERSION 4.0)
project(AllIn1)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(cppParse)

add_library(allin1_common STATIC
    src/common/string_utils.cpp
    src/common/platform.cpp
)
set_target_properties(allin1_common PROPERTIES PREFIX "")
target_include_directories(allin1_common PUBLIC include)

add_library(allin1_io STATIC
    src/io/io.cpp
    src/io/create.cpp
    src/io/symlink.cpp
    src/io/shortcut.cpp
)
set_target_properties(allin1_io PROPERTIES PREFIX "")
target_include_directories(allin1_io PUBLIC include cppParse/include)
target_link_libraries(allin1_io PUBLIC allin1_common cppParse)

add_executable(AllIn1 src/main.cpp)
target_link_libraries(AllIn1 PRIVATE allin1_io cppParse)
target_include_directories(AllIn1 PUBLIC include cppParse/include)
