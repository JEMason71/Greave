# CMake build file for Greave, intended for use with MinGW/GCC on Windows and GCC on Linux.
# Copyright (c) 2021 Raine "Gravecat" Simmons and the Greave contributors. Licensed under the GNU Affero General Public License v3 or any later version.

cmake_minimum_required(VERSION 3.13)

# LodePNG source files
set(LODEPNG_CPPS
  bmp2png.cpp
  lodepng.cpp
)

# We'll compile LodePNG as a static library, and link it into the project.
add_library(lodepng STATIC ${LODEPNG_CPPS})

# Include directories.
target_include_directories(lodepng PRIVATE
  "${CMAKE_SOURCE_DIR}/src/3rdparty"
  "${CMAKE_SOURCE_DIR}/src/3rdparty/LodePNG"
)
