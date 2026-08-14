cmake_minimum_required(VERSION 3.20)
project(gsa CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(GSA_BUILD_EXAMPLES "Build the API demo (examples/main.cpp)" ON)
option(GSA_BUILD_TESTS "Build and register the CTest suite" ON)

add_library(gsa STATIC src/gsa.cpp)
add_library(gsa::gsa ALIAS gsa)
target_include_directories(
    gsa PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party>)
target_compile_options(gsa PRIVATE -Wall -Wextra)

# Fast warning-free syntax check of all sources.
set(GSA_SOURCES src/gsa.cpp examples/main.cpp
    tests/test_main.cpp tests/tasks/test_history.cpp tests/tasks/test_stats.cpp
    tests/tasks/test_determinism.cpp tests/tasks/test_modes.cpp
    tests/tasks/test_thread_safety.cpp)
add_custom_target(check-gsa
    COMMAND ${CMAKE_CXX_COMPILER} -Wall -Wextra -Werror -std=c++20 -fsyntax-only
            -I${CMAKE_CURRENT_SOURCE_DIR}/include
            -I${CMAKE_CURRENT_SOURCE_DIR}/third_party
            ${CMAKE_CURRENT_SOURCE_DIR}/src/gsa.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/examples/main.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_main.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/tasks/test_history.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/tasks/test_stats.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/tasks/test_determinism.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/tasks/test_modes.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/tasks/test_thread_safety.cpp
    DEPENDS ${GSA_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Google-casing naming lint via clang-tidy; check-gsa must stay warning-free too.
find_program(GSA_CLANG_TIDY clang-tidy)
if(GSA_CLANG_TIDY)
  add_custom_target(check-naming
      COMMAND ${GSA_CLANG_TIDY} src/gsa.cpp
              -p ${CMAKE_BINARY_DIR}
              --extra-arg=-Wno-unknown-pragmas
      DEPENDS gsa ${GSA_SOURCES}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(GSA_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(GSA_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()