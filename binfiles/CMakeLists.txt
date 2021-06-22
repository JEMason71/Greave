# CMake build file for Greave, intended for use with MinGW/GCC on Windows and GCC on Linux. Support for OS X, Visual Studio, clang, etc. is not currently available.
# For MinGW/MSYS builds, be sure to specify -G"MinGW Makefiles" when setting up the build folder.
# Copyright (c) 2020-2021 Raine "Gravecat" Simmons and the Greave contributors. Licensed under the GNU Affero General Public License v3 or any later version.

cmake_minimum_required(VERSION 3.0)


# This mess here copies all the game's static data (mostly YAML files) into the bin folder where the compiled binary lives.
add_custom_target(greave_data ALL
  DEPENDS greave
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/data"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/data/app-icon.ico" "${CMAKE_BINARY_DIR}/bin/data/"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/data/tune.yml" "${CMAKE_BINARY_DIR}/bin/data/"
)

# Windows needs a whole mess of DLLs.
if(TARGET_WINDOWS)
  if(TARGET_64)
    add_custom_target(dll ALL
      DEPENDS greave
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win64/BearLibTerminal.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win64/libgcc_s_seh-1.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win64/libstdc++-6.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win64/libwinpthread-1.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win64/nvdaControllerClient64.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win64/pdcurses.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win64/SAAPI64.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win64/Tolk.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win64/yaml-cpp.dll" "${CMAKE_BINARY_DIR}/bin"
    )
  else()
    add_custom_target(dll ALL
      DEPENDS greave
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win32/BearLibTerminal.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win32/dolapi32.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win32/libgcc_s_dw2-1.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win32/libstdc++-6.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win32/libwinpthread-1.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win32/nvdaControllerClient32.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win32/pdcurses.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win32/SAAPI32.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win32/Tolk.dll" "${CMAKE_BINARY_DIR}/bin"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/win32/yaml-cpp.dll" "${CMAKE_BINARY_DIR}/bin"
    )
  endif(TARGET_64)
endif(TARGET_WINDOWS)

# Copy the license files across, so we can easily just zip up the bin folder for distribution.
add_custom_target(licenses ALL
  DEPENDS greave
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/licenses"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/binfiles/readme.txt" "${CMAKE_BINARY_DIR}/bin/"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/LICENSE" "${CMAKE_BINARY_DIR}/bin/licenses/agpl-3.0.txt"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/licenses/bearlibterminal.txt" "${CMAKE_BINARY_DIR}/bin/licenses/"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/licenses/freetype.txt" "${CMAKE_BINARY_DIR}/bin/licenses/"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/licenses/nanojpeg.txt" "${CMAKE_BINARY_DIR}/bin/licenses/"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/licenses/picopng.txt" "${CMAKE_BINARY_DIR}/bin/licenses/"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/licenses/tolk.txt" "${CMAKE_BINARY_DIR}/bin/licenses/"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/licenses/yaml-cpp.txt" "${CMAKE_BINARY_DIR}/bin/licenses/"
)

if(TARGET_LINUX)
  if(TARGET_64)
    add_custom_target(so ALL
	  DEPENDS greave
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/lib/lin64/libBearLibTerminal.so" "${CMAKE_BINARY_DIR}/bin"
    )
  else()
    add_custom_target(so ALL
	  DEPENDS greave
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/lib/lin32/libBearLibTerminal.so" "${CMAKE_BINARY_DIR}/bin"
    )
  endif(TARGET64)
endif(TARGET_LINUX)
