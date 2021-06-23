# BUILD INSTRUCTIONS

## Windows

**Requirements:** [CMake](https://cmake.org/) version 3.13 or newer, MinGW ([MinGW-w64](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/) recommended, [MSYS2](https://www.msys2.org/) should also work).

The build directory can be anywhere, but for the purposes of this example, use the `build` folder within the project's root directory. From a command prompt or MinGW shell (*not* MSYS shell) within the `build` folder, run the following command:

**MinGW-w64:** `cmake -G"MinGW Makefiles" ..`

**MSYS2:** `cmake -G"MSYS Makefiles" ..`

Assuming there are no errors, run `mingw32-make -j` (or `make -j` for MSYS2) once the CMake configuration is complete. The binary should appear within the `bin` subfolder, along with its required DLLs and static data files.

To run the game in PDCurses (console mode), make a `userdata` folder within the `bin` folder if one does not yet exist, then create the file `bin\userdata\tune.yml`. Within this file, write: `terminal: curses`. The next time you run `greave.exe`, it should now be in a text console.

**Note:** The prebuilt MinGW .dll, .lib and .a files provided for BearLibTerminal, libgcc/libstdc++/libwinpthread, PDCurses and yaml-cpp are built with MinGW x86_64-posix-seh and i686-posix-dwarf. You may need to compile your own version of these files if you are using a different MinGW configuration.

## Linux

**Requirements:** [CMake](https://cmake.org/) version 3.13 or newer, and the development packages `libncurses5-dev`, `libsqlite3-dev`, `libx11-dev` (or the equivalent packages for your distro).

The build directory can be anywhere, but for the purposes of this example, use the `build` folder within the project's root directory. From within the `build` folder, run the following command: `cmake ..`

Assuming there are no errors, run `make -j` once the CMake configuration is complete. The binary should appear within the `bin` subfolder, along with its static data files.

To run the game in NCurses (console mode), make a `userdata` folder within the `bin` folder if one does not yet exist, then create the file `bin/userdata/tune.yml`. Within this file, write: `terminal: curses`. The next time you run the Greave binary, it should now be in a text console.

## Other Platforms

macOS X and other platforms are unsupported and untested at the time of writing. If you are able to compile Greave on another target platform and require changes to the build environment, please open a pull request.
