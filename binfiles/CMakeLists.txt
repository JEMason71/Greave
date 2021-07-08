# CMake build file for Greave, intended for use with MinGW/GCC on Windows and GCC on Linux. Support for OS X, Visual Studio, clang, etc. is not currently available.
# For MinGW/MSYS builds, be sure to specify -G"MinGW Makefiles" when setting up the build folder.
# Copyright (c) 2020-2021 Raine "Gravecat" Simmons and the Greave contributors. Licensed under the GNU Affero General Public License v3 or any later version.

cmake_minimum_required(VERSION 3.0)


# This mess here copies all the game's static data (mostly YAML files) into the bin folder where the compiled binary lives.
add_custom_target(greave_data ALL
  DEPENDS greave
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/binfiles/data" "${CMAKE_BINARY_DIR}/bin/data"
)

# Windows needs a whole mess of DLLs.
if(TARGET_WINDOWS)
  if(TARGET_64)
    add_custom_target(dll ALL
      DEPENDS greave
      COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/binfiles/win64" "${CMAKE_BINARY_DIR}/bin"
    )
  else()
    add_custom_target(dll ALL
      DEPENDS greave
      COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/binfiles/win32" "${CMAKE_BINARY_DIR}/bin"
    )
  endif(TARGET_64)
endif(TARGET_WINDOWS)

# Copy the license files across, so we can easily just zip up the bin folder for distribution.
add_custom_target(licenses ALL
  DEPENDS greave
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/licenses" "${CMAKE_BINARY_DIR}/bin/licenses"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/readme.txt" "${CMAKE_BINARY_DIR}/bin/"
)
