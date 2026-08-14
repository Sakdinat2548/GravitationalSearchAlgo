cmake_minimum_required(VERSION 3.20)
project(gsa VERSION 0.1.0 LANGUAGES CXX)

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
option(GSA_BUILD_INSTALL "Generate install/export rules (find_package)" ON)

add_library(gsa STATIC src/gsa.cpp)
add_library(gsa::gsa ALIAS gsa)
target_include_directories(
    gsa PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/third_party>
    $<INSTALL_INTERFACE:include>)
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

if(GSA_BUILD_INSTALL)
  include(GNUInstallDirs)
  include(CMakePackageConfigHelpers)

  install(TARGETS gsa EXPORT gsaTargets
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  install(FILES third_party/XoshiroCpp.hpp
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  install(EXPORT gsaTargets
      NAMESPACE gsa::
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gsa)

  configure_package_config_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/gsaConfig.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/gsaConfig.cmake
      INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gsa)
  write_basic_package_version_file(
      ${CMAKE_CURRENT_BINARY_DIR}/gsaConfigVersion.cmake
      VERSION ${PROJECT_VERSION}
      COMPATIBILITY SameMajorVersion)
  install(FILES
      ${CMAKE_CURRENT_BINARY_DIR}/gsaConfig.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/gsaConfigVersion.cmake
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/gsa)
endif()