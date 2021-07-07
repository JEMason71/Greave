# CMake build file for Greave, intended for use with MinGW/GCC on Windows and GCC on Linux.
# Copyright (c) 2020-2021 Raine "Gravecat" Simmons and the Greave contributors. Licensed under the GNU Affero General Public License v3 or any later version.

cmake_minimum_required(VERSION 3.13)
if(${CMAKE_VERSION} VERSION_LESS "3.14")
  message("------------------------------------------------------------------------------")
  message("PLEASE NOTE: If you are using CMake version 3.13, CMake *may* be unable to")
  message("locate SQLite3. It should be okay regardless, but please consider upgrading to")
  message("CMake 3.14 or a newer version. https://github.com/Gravecat/Greave/issues/17")
  message("------------------------------------------------------------------------------")
endif()


# SQLiteCpp source files.
set(SQLITE_CPP_CPPS
  Backup.cpp
  Column.cpp
  Database.cpp
  Exception.cpp
  Statement.cpp
  Transaction.cpp
)

# This silences a compiler warning, since the project is usually compiled with -Wall -Wextra.
set_source_files_properties(${SQLITE_CPP_CPPS} PROPERTIES COMPILE_FLAGS "-Wno-stringop-truncation")

# We'll compile SQLiteCpp as a static library, and link it into the project.
add_library(sqlite_cpp STATIC ${SQLITE_CPP_CPPS})

# Include directories, also points directly to sqlite3.
target_include_directories(sqlite_cpp PRIVATE
  "${CMAKE_SOURCE_DIR}/src/3rdparty"
  "${CMAKE_SOURCE_DIR}/src/3rdparty/sqlite3"
)

# Platform-specific fuckery.
if(TARGET_WINDOWS)
  set(SQLITE_LIBRARIES sqlite3.dll)
else()
  set(SQLITE_LIBRARIES sqlite3 dl)
  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14")
    find_package(SQLite3 REQUIRED)
  endif()
  if(SQLite3_VERSION VERSION_LESS "3.19")
    set_target_properties(greave PROPERTIES COMPILE_FLAGS "-DSQLITECPP_HAS_MEM_STRUCT")
  endif()
endif(TARGET_WINDOWS)

# Link libraries. Platform-specific stuff is set above.
target_link_libraries(sqlite_cpp
  ${SQLITE_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)
