cmake_minimum_required(VERSION 3.20)
project(gsa CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(EXISTS "${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake")
  include(${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake)
elseif(EXISTS "${CMAKE_BINARY_DIR}/conan_toolchain.cmake")
  include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
endif()

find_package(nlohmann_json 3.11.2 CONFIG REQUIRED)
find_package(xoshiro-cpp 1.1 CONFIG REQUIRED)

option(GSA_BUILD_DEMO "Build the API demo (examples/demo.cpp)" ON)
option(GSA_BUILD_TESTS "Build and register the CTest suite" ON)
option(GSA_BUILD_MAIN "Build the main executable (examples/main.cpp)" ON)

add_library(gsa INTERFACE)
add_library(gsa::gsa ALIAS gsa)
target_include_directories(
    gsa INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
target_link_libraries(gsa INTERFACE nlohmann_json::nlohmann_json xoshiro-cpp::xoshiro-cpp)
target_compile_options(gsa INTERFACE -Wall -Wextra)

set(GSA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/tasks/test_history.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/tasks/test_stats.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/tasks/test_determinism.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/tasks/test_modes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/tasks/test_thread_safety.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/tasks/test_median.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/tasks/test_convergence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/tasks/test_validation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/tasks/test_snapshots.cpp)

add_custom_target(check-gsa
    COMMAND ${CMAKE_CXX_COMPILER} -Wall -Wextra -Werror -std=c++20 -fsyntax-only
            -I${CMAKE_CURRENT_SOURCE_DIR}/src
            -I${nlohmann_json_INCLUDE_DIRS_RELEASE}
            -I${xoshiro-cpp_INCLUDE_DIRS_RELEASE}
            ${GSA_SOURCES}
    DEPENDS ${GSA_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

find_program(GSA_CLANG_TIDY clang-tidy)
if(GSA_CLANG_TIDY)
  add_custom_target(check-naming
      COMMAND ${GSA_CLANG_TIDY} src/gsa/gsa.hpp src/gsa/stats.hpp
              -p ${CMAKE_BINARY_DIR}
              --extra-arg=-Wno-unknown-pragmas
      DEPENDS ${GSA_SOURCES}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(GSA_BUILD_DEMO)
  add_executable(demo examples/demo.cpp)
  target_link_libraries(demo PRIVATE gsa::gsa)
  target_compile_options(demo PRIVATE -Wall -Wextra)
endif()

if(GSA_BUILD_MAIN)
  add_executable(main examples/main.cpp)
  target_link_libraries(main PRIVATE gsa::gsa nlohmann_json::nlohmann_json)
  target_compile_options(main PRIVATE -Wall -Wextra)
endif()

if(GSA_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
