# Codesoc Cubed

A small voxel sandbox for the Coding Society's C++ Introductory Workshop.
Everytime the program is run, a random world is genereated consistenting of gentle hills.

## Build and Run

### Prerequisites

You will need the following:
- C++20 compiler
- CMake 3.25 or newer
- Support for OpenGL 3.3

CMake downloads the official Raylib 6.0 release during configuration.

The prepared Linux lab machines have the required toolchain installed.

### Instructions

```sh
cmake -S . -B build/debug/ -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/debug/
./build/debug/codesoc-cubed
```

Use `--seed 42` to generate the same terrain on repeated runs.
Run `./build/debug/codesoc-cubed --help` for the supported options.

## Controls

| Input | Action |
| --- | --- |
| WASD and mouse | Move and look |
| Space | Jump, or rise while flying |
| Double Space | Toggle flight |
| Left Shift | Move faster |
| Left Ctrl | Descend while flying |
| Left and right mouse buttons | Mine and place blocks |
| Middle mouse button | Select the targeted block |
| 1–9 and mouse wheel | Select a hotbar slot |
| F3 | Toggle position, seed, chunk and frame information |
| H | Toggle the controls card |
| F11 | Toggle borderless fullscreen |
| F12 | Save a screenshot |
| Escape or Q | Quit immediately |

The first three hotbar slots contain grass, dirt and stone.
The remaining six slots are empty and cannot place blocks.
Raylib saves F12 screenshots in the working directory as `screenshot000.png`, `screenshot001.png` and so on.

## World and Code

Terrain is generated from the seed and world coordinates, so revisiting an untouched area reproduces the same hills.
Chunks are 16 × 16 blocks horizontally and 64 blocks tall.
Nearby chunks load a few at a time, while distant untouched chunks unload.
Edited chunks remain in memory for the current run and are never written to disk.

- `src/world.*` contains terrain generation, chunk streaming, movement, collision and targeting
- `src/renderer.*` builds and draws visible chunk faces
- `src/application.*` handles the window, input, hotbar and frame loop
- `tests/world_tests.cpp` checks the graphics-independent world behaviour

## C++ Style

CI uses clang-format 18 and clang-tidy 18.
Keep lines within 100 columns and run `clang-format-18 -i src/*.cpp src/*.hpp tests/*.cpp` after editing C++ files.
Write function definitions on multiple lines and leave one blank line between logical blocks of work.
The logical-block spacing is a review guideline; the other mechanical rules live in `.clang-format` and `.clang-tidy`.
Use Doxygen-style comments for public behavior and non-obvious project rules, with tags only when they add information beyond the declaration.
Keep comments accurate when changing the behavior they describe, and review them for usefulness to a newcomer.

## Verification

```sh
ctest --test-dir build --output-on-failure
./build/codesoc-cubed --seed 42 --smoke-test
clang-format-18 --dry-run --Werror src/*.cpp src/*.hpp tests/*.cpp
clang-tidy-18 -p build --warnings-as-errors='*' src/*.cpp tests/*.cpp
```

The smoke test opens a hidden graphics window, draws the world for a short run and checks startup, movement, flight and editing without writing an image.
For a graphics-free test build, configure with `-DCODESOC_BUILD_GAME=OFF`.
