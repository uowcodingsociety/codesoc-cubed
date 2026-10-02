# Lab

## Section 0

In this section, we still start by cloning the repository and exploring the world.

Open the terminal and run the following commands:

1. Clone the repo:

```sh
git clone https://github.com/uowcodingsociety/codesoc-cubed.git
```

This will copy the starter code onto your system.

2. Go into that project:

```sh
cd codesoc-cubed/
```

3. Configure a CMake Release build:

```sh
cmake -S . -B build/release/ -G Ninja -DCMAKE_BUILD_TYPE=Release
```

4. Build the project:

```sh
cmake --build build/release/
```

5. Run the project:

```sh
./build/release/codesoc-cubed
```

Explore with WASD and the mouse.
Press Space to jump, double-tap Space to fly, use Shift to move faster and use Ctrl to descend while flying.
Mine with the left mouse button and place the selected block with the right mouse button.
Slots 1–3 contain grass, dirt and stone.
Select these with the number keys.
Press F3 to toggle a compact panel in the top-left corner showing a white FPS counter and the world seed.
The panel starts hidden.
Escape or Q quits immediately.

## Section 1

Let's starting writing some code now.

### Setup

We'll create a Debug build to help us with debugging if we need it.

1. Configure a CMake Debug build:

```sh
cmake -S . -B build/debug/ -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

2. Build the project:

```sh
cmake --build build/debug/
```

3. Run the project:

```sh
cmake --build build/debug/
```

4. Open the code in your favourite editor. We recommend VS Code.

```sh
code .
```

### Player

Find `walkingSpeed` in `src/world.cpp` and change it from `4.5f` to `6.0f`.
Build and run again with `--seed 42`, then compare walking speed with the first run.

### Terrain Generation

Next, find `broadTerrainHeight` in the same file and lower it from `8.0f` to `5.0f`.
Build and run again with the same seed to see gentler hills.
