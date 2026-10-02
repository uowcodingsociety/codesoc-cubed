# Codesoc Cubed

A small voxel sandbox for the Coding Society's C++ Introductory Workshop.
Every time the program is run, a random world is generated consisting of gentle hills.

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
cmake -S . -B build/debug/ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
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
| F3 | Toggle FPS and world seed |
| F11 | Toggle borderless fullscreen |
| F12 | Save a screenshot |
| Escape or Q | Quit immediately |

The first three hotbar slots contain grass, dirt and stone.
The remaining six slots are empty and cannot place blocks.
F3 shows a compact panel in the top-left corner with a white FPS counter and the world seed.
The panel starts hidden.
Raylib saves F12 screenshots in the working directory as `screenshot000.png`, `screenshot001.png` and so on.

## World and Code

Terrain is generated from the seed and world coordinates, so revisiting an untouched area reproduces the same hills.
Chunks are 16 × 16 blocks horizontally and 64 blocks tall.
Nearby chunks load a few at a time, while distant untouched chunks unload.
Edited chunks remain in memory for the current run and are never written to disk.

- `src/blocks.hpp` defines block names, RGB colours and optional face overrides
- `src/world.*` contains terrain generation, chunk streaming, movement, collision and targeting
- `src/renderer.*` builds and draws visible chunk faces
- `src/application.*` handles the window, input, hotbar and frame loop
- `tests/block_tests.cpp` checks block definitions and face colours
- `tests/world_tests.cpp` checks the graphics-independent world behaviour

## C++ Style

CI uses clang-format 21 and clang-tidy 21.
Run `clang-format-21 -i src/*.cpp src/*.hpp tests/*.cpp` after editing C++ files.

## Verification

```sh
ctest --test-dir build/debug/ --output-on-failure
./build/debug/codesoc-cubed --seed 42 --smoke-test
clang-format-21 --dry-run --Werror src/*.cpp src/*.hpp tests/*.cpp
clang-tidy-21 -p build/debug/ --warnings-as-errors='*' src/*.cpp tests/*.cpp
```

The smoke test opens a hidden graphics window, draws the world for a short run and checks startup, movement, flight and editing without writing an image.
For a graphics-free test build, configure with `-DCODESOC_BUILD_GAME=OFF`.
