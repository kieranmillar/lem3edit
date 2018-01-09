# lem3edit
A Lemmings 3 Level Editor

Originally by Carl Reinke at https://bitbucket.org/mindless/lemmings-tools

This tool allows you to edit levels for Lemmings 3 (also known as Lemmings Chronicles or All New World of Lemmings) so you can create and share your own custom content.

This requires a copy of Lemmings 3, a copyrighted third-party game that is not distributed with lem3edit.

Build instructions
------------------

* Place all files and directories of Lemmings 3 in lem3edit's root directory.
* Install SDL2 and its SDL_ttf 2.0 addon.
* Optionally, install [CMake](https://cmake.org/); alternatively,
    build manually (see further down).

**With CMake,** start in lem3edit's root directory, then enter:
```
mkdir build
cd build
cmake ..
make
```
This produces an executable `lem3edit` or `lem3edit.exe` in `build/`.
This executable should be run using lem3edit's root directory
for its working directory, e.g., by `cd ..`, then `build/lem3edit`.
If something goes wrong, delete the entire `build/` directory and start over.

**Manual build:** If you do not wish to use CMake,
give to your C++ compiler all `*.cpp` files in `src/` and link to
`SDL2main` and `SDL2`.

