cmake_minimum_required(VERSION 3.20)
project(gsa CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

add_library(gsa STATIC gsa.cpp)
target_include_directories(gsa PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(gsa PRIVATE -Wall -Wextra)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE gsa)
target_compile_options(main PRIVATE -Wall -Wextra)

add_executable(gsa_test
    test/test_main.cpp
    test/test_history.cpp
    test/test_stats.cpp
    test/test_determinism.cpp
    test/test_modes.cpp
)
target_link_libraries(gsa_test PRIVATE gsa)
target_compile_options(gsa_test PRIVATE -Wall -Wextra)

# Fast warning-free syntax check of all sources (mirrors the AGENTS.md lint step).
set(GSA_SOURCES gsa.cpp main.cpp
    test/test_main.cpp test/test_history.cpp test/test_stats.cpp
    test/test_determinism.cpp test/test_modes.cpp)
add_custom_target(check-gsa
    COMMAND ${CMAKE_CXX_COMPILER} -Wall -Wextra -Werror -std=c++20 -fsyntax-only
            ${CMAKE_CURRENT_SOURCE_DIR}/gsa.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/test/test_main.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/test/test_history.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/test/test_stats.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/test/test_determinism.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/test/test_modes.cpp
    DEPENDS ${GSA_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()
add_test(NAME gsa_history COMMAND gsa_test history_size)
add_test(NAME gsa_stats COMMAND gsa_test history_stats)
add_test(NAME gsa_determinism COMMAND gsa_test determinism)
add_test(NAME gsa_modes COMMAND gsa_test modes)
