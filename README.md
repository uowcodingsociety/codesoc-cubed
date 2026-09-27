# Codesoc Cubed

Codesoc Cubed is a small C++ voxel sandbox for the CodeSoc Welcome Week lab.
Each run starts in a new world of gentle hills.
The terrain extends horizontally as the player travels, and changes last only until the program exits.

## Build and Run

You need a C++20 compiler, CMake 3.25 or newer and OpenGL 3.3 support.
CMake downloads the official Raylib 6.0 release during configuration with its default F12 screenshot shortcut enabled.
The prepared Linux lab machines have the required toolchain installed.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --parallel
./build/codesoc-cubed
```

Use `--seed 42` to generate the same terrain on repeated runs.
Run `./build/codesoc-cubed --help` for the supported options.

On Debian or Ubuntu, the desktop development packages are available with:

```sh
sudo apt install build-essential cmake git libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev
```

On macOS, install the Xcode command line tools and CMake, then use the same build commands.
On Windows, use a Visual Studio C++ development shell, run `cmake --build build --config Debug` and start `build/Debug/codesoc-cubed.exe`.
Linux is the first tested platform for the lab.

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
There is no automatic rescue after falling into the void, although double Space can start flight during a fall.
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
When a call, initializer or function declaration wraps, put each argument, element or parameter on its own line as closely as clang-format allows.
Long conditions should keep each top-level `&&` or `||` test visually distinct; clang-format 18 approximates this rather than enforcing every case.
Use braces for every `if`, `else`, `for`, `while` and `do` body, including single-statement bodies.
Write function definitions on multiple lines and leave one blank line between logical blocks of work.
The logical-block spacing is a review guideline; the other mechanical rules live in `.clang-format` and `.clang-tidy`.

## Verification

```sh
ctest --test-dir build --output-on-failure
./build/codesoc-cubed --seed 42 --smoke-test
clang-format-18 --dry-run --Werror src/*.cpp src/*.hpp tests/*.cpp
clang-tidy-18 -p build --warnings-as-errors='*' src/*.cpp tests/*.cpp
```

The smoke test opens a hidden graphics window, draws the world for a short run and checks startup, movement, flight and editing without writing an image.
For a graphics-free test build, configure with `-DCODESOC_BUILD_GAME=OFF`.
