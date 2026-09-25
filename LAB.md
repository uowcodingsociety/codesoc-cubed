# Codesoc Cubed Welcome Week Lab

Work through Stages 0–2 at your own pace during the 90-minute session.
You need some experience with variables, loops and functions, but you do not need to know C++ already.
Stage 3 is optional if you finish early.
Keep your changes locally; there is no submission.

## Stage 0: Clone, Build and Explore

The organiser will replace `REPOSITORY_URL` with the public GitHub address before the event.
The Linux lab machines already have Git, CMake, a C++ compiler and the required graphics libraries.

```sh
git clone REPOSITORY_URL
cd codesoc-cubed
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/codesoc-cubed --seed 42
```

Explore with WASD and the mouse.
Press Space to jump, double-tap Space to fly, use Shift to move faster and use Ctrl to descend while flying.
Mine with the left mouse button and place the selected block with the right mouse button.
Slots 1–3 contain grass, dirt and stone; slots 4–9 are deliberately empty.
Press F3 to see your position and current chunk, then try crossing a chunk boundary.
Escape or Q quits immediately.

Checkpoint: the build and tests pass, and you can move, mine, place and fly in the world.

If you are on a personal laptop, follow the platform notes in [README.md](README.md) before the session.
CMake may need network access on the first configure to download Raylib 6.0.

## Stage 1: Tune the World

Find `walkingSpeed` in `src/world.cpp` and change it from `4.5f` to `6.0f`.
Build and run again with `--seed 42`, then compare walking speed with the first run.
Next, find `broadTerrainHeight` in the same file and lower it from `8.0f` to `5.0f`.
Build and run again with the same seed to see gentler hills.

Checkpoint: your changes compile, the terrain is visibly flatter and the tests still pass.
If a later experiment makes a test fail, read the failure before deciding whether to restore the value or adjust the test's expectation.

## Stage 2: Add Stone Outcrops

The generator currently puts grass on every natural surface block.
Add deterministic patches of exposed stone using only the existing grass and stone materials.
Work in the surface-material helper in `src/world.cpp`, which is shared by streamed chunks and on-demand block queries.

Hints:

1. A decision based on `x`, `z` and the world seed gives the same result regardless of chunk loading order
2. `smoothValueNoise` can create neighbouring cells with similar values, producing patches instead of isolated dots
3. Choose a separate seed offset for the outcrops so their pattern differs from the hill pattern
4. Leave the dirt and stone layers beneath the surface unchanged

Build and run with `--seed 42` after each small change.
Use F3 and flight to explore more than one chunk, including negative X or Z coordinates.
Run the tests and check visually that the same seed produces the same outcrops after restarting.
Your implementation should still be correct when an untouched chunk unloads and later regenerates.

Checkpoint: stone outcrops appear across chunk boundaries, the build succeeds and all tests pass.

If you have time, choose one bounded follow-up:

- Show the player's local X/Z position within the current chunk in the F3 debug display
- Improve the control hints for a narrower game window and check them at the minimum window size
- Change the outcrop density through one named constant and compare two fixed-seed runs

## Stage 3: Try a Larger Idea

Stage 3 is open-ended and is not required to finish the lab.
You could add another terrain region, design a new material, change how chunks are drawn or build a new interaction.
State what you want to change, make one small step, then build and test before continuing.
Ask a facilitator for a second opinion if your idea touches several files.
