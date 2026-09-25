cmake_minimum_required(VERSION 4.0)

find_package(cmake-bare REQUIRED PATHS node_modules/cmake-bare)
find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ggwave is C++, share the app's C++ runtime on Android
set(ANDROID_STL c++_shared CACHE STRING "")

# 16 KB pages, which newer Android devices require
set(ANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES ON CACHE BOOL "")

project(bare_ggwave C CXX)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GGWAVE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GGWAVE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GGWAVE_SUPPORT_SDL2 OFF CACHE BOOL "" FORCE)
set(GGWAVE_ALL_WARNINGS OFF CACHE BOOL "" FORCE)

fetch_package("github:ggerganov/ggwave#060aec7")

add_bare_module(bare_ggwave)

target_sources(
  ${bare_ggwave}
  PRIVATE
    binding.cc
)

target_link_libraries(
  ${bare_ggwave}
  PRIVATE
    ggwave
)
