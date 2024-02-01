cmake_minimum_required(VERSION 3.22)

project(Equalizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(CPM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON)
set(JUCE_BUILD_EXTRAS OFF)
set(JUCE_BUILD_EXAMPLES OFF)

file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.2/CPM.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake
  # EXPECTED_HASH
  # SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494
)

include(${CPM_DIR}/CPM.cmake)

cpmaddpackage(
  NAME
  JUCE
  GITHUB_REPOSITORY
  juce-framework/JUCE
  GIT_TAG
  develop
  SOURCE_DIR
  ${LIB_DIR}/juce)

cpmaddpackage(
  NAME
  TRACER
  GITHUB_REPOSITORY
  fabrizioperria/tracer
  GIT_TAG
  main
  SOURCE_DIR
  ${LIB_DIR}/tracer)

cpmaddpackage(
  NAME
  GOOGLETEST
  GITHUB_REPOSITORY
  google/googletest
  VERSION
  1.13.0
  SOURCE_DIR
  ${LIB_DIR}/googletest
  OPTIONS
  "INSTALL_GTEST OFF"
  "gtest_force_shared_crt ON")

# enable_testing()

if(MSVC)
  add_compile_options(/Wall /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(Source)
# add_subdirectory(test)
