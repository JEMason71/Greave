# CMake build file for Greave, intended for use with MinGW/GCC on Windows and GCC on Linux. Support for OS X, Visual Studio, clang, etc. is not currently available.
# For MinGW/MSYS builds, be sure to specify -G"MinGW Makefiles" when setting up the build folder.
# Copyright (c) 2020-2021 Raine "Gravecat" Simmons and the Greave contributors. Licensed under the GNU Affero General Public License v3 or any later version.

cmake_minimum_required(VERSION 3.13)


# Non-platform-specific stuff.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -pedantic-errors")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
set(CMAKE_CXX_FLAGS_RELEASE  "-O2")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s")
#set(SQLITECPP_LIBRARIES sqlite_cpp)


# Main platform-specific settings.
if(TARGET_WINDOWS)
  
  # Platform-specific libraries.
  set(OS_LIBRARIES mingw32)
  set(TOLK_LIBRARIES Tolk)
  add_definitions(-DGREAVE_TOLK)
  set(YAMLCPP_LIBRARIES yaml-cpp.dll)
  set(CURSES_LIBRARIES pdcurses)
  
  # Application icon
  enable_language(RC)
  set(CMAKE_RC_COMPILER_INIT windres)
  set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
  set(GREAVE_RC core/app-icon.rc)
  
else()
  
  # Platform-specific libraries.
  set(OS_LIBRARIES ${X11_LIBRARIES})
  set(YAMLCPP_LIBRARIES yaml-cpp)
  set(CURSES_LIBRARIES ncurses)
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s -Wl,-rpath='${ORIGIN}'")
  
endif(TARGET_WINDOWS)

# Source files.
set(GREAVE_CPPS
  core/core.cpp
  core/guru.cpp
  core/message.cpp
  core/tune.cpp
  core/utility.cpp
  core/world.cpp
  terminal/terminal-blt.cpp
  terminal/terminal-curses.cpp
)


# Binary file. GREAVE_RC should be blank for non-Windows builds.
add_executable(greave ${GREAVE_CPPS} ${GREAVE_RC})


# Include directories. 3rdparty is included here, because otherwise yaml-cpp gets unhappy.
target_include_directories(greave PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/src/3rdparty"
)


# Platform-specific library directories.
if(TARGET_WINDOWS)
  if(TARGET_32)
    target_link_directories(greave PRIVATE "${CMAKE_SOURCE_DIR}/lib/win32")
  else()
    target_link_directories(greave PRIVATE "${CMAKE_SOURCE_DIR}/lib/win64")
  endif(TARGET_32)
else()
  if(TARGET_32)
    target_link_directories(greave PRIVATE "${CMAKE_SOURCE_DIR}/lib/lin32")
  else()
    target_link_directories(greave PRIVATE "${CMAKE_SOURCE_DIR}/lib/lin64")
  endif(TARGET_32)
endif(TARGET_WINDOWS)


# Link libraries. Platform-specific stuff should be set in the main platform-specific section near the top.
target_link_libraries(greave
  ${OS_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${CURSES_LIBRARIES}
  #${SQLITECPP_LIBRARIES}
  ${TOLK_LIBRARIES}
  ${YAMLCPP_LIBRARIES}
  BearLibTerminal
)


# Post-build, make a 'bin' folder and copy the binary file in there.
add_custom_command(TARGET greave POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:greave> "${CMAKE_BINARY_DIR}/bin"
)


# SQLiteCpp comes with source files as well as headers, so we'll compile it separately.
#add_subdirectory(3rdparty/SQLiteCpp)
