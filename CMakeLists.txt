cmake_minimum_required(VERSION 3.20)
project(gsa CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -flto")
endif()

add_library(gsa STATIC gsa.cpp)
target_include_directories(gsa PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(gsa PRIVATE -Wall -Wextra)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE gsa)
target_compile_options(main PRIVATE -Wall -Wextra)

add_executable(bench bench/bench.cpp)
target_link_libraries(bench PRIVATE gsa)
target_compile_options(bench PRIVATE -Wall -Wextra)

# Fast syntax check mirroring the AGENTS.md lint step.
add_custom_target(check-gsa
    COMMAND ${CMAKE_CXX_COMPILER} -Wall -Wextra -std=c++20 -fsyntax-only
            ${CMAKE_CURRENT_SOURCE_DIR}/gsa.cpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
