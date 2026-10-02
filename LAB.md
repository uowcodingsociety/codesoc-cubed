# Lab

Start by exploring the game, then change its movement, blocks and terrain.
Work through the tasks in order and get as far as you can.
For each change, predict what will happen and check the result in the game.

C++, CMake, raylib and Git may be new to you, and that's expected.
If a code example or task is unclear, try looking up the relevant documentation.

Examples:

- [C++](https://cppreference.com/)
- [CMake](https://cmake.org/cmake/help/v3.25/index.html): We use CMake 3.25 as the minimum version for compatibility with DCS machines. Any CMake version 3.25 or newer should work with this project.
- [raylib](https://www.raylib.com/)

If the documentation is unclear, ask one of the lab tutors for help.
The lab sheet should give you enough guidance to complete the tasks, but the documentation can help if you need more explanation or want to learn more.

## Get Started

Open a terminal and run these commands.

> Note: If you are using a DCS machine, you will need to load the correct compiler first:

```sh
module load GCC/13.3.0
```

1. Clone the starter code:

```sh
git clone https://github.com/uowcodingsociety/codesoc-cubed.git
```

2. Go into the project:

```sh
cd codesoc-cubed/
```

3. Configure a Release build:

```sh
cmake -S . -B build/release/ -G Ninja -DCMAKE_BUILD_TYPE=Release
```

4. Build the project:

```sh
cmake --build build/release/
```

5. Run the game:

```sh
./build/release/codesoc-cubed
```

Take a look around before changing anything.

- Move with WASD and look around with the mouse
- Press Space to jump, double-tap Space to toggle flight and hold Shift to move faster
- Hold Ctrl to descend while flying
- Mine with the left mouse button and place blocks with the right mouse button
- Select grass, dirt or stone with keys 1–3
- Press F3 to show or hide the FPS counter and world seed
- Press Escape or Q to quit

## Set Up for Coding

Create a Debug build to help investigate problems as you code.

1. Configure the build:

```sh
cmake -S . -B build/debug/ -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

2. Build it:

```sh
cmake --build build/debug/
```

3. Run it:

```sh
./build/debug/codesoc-cubed
```

4. Open the code in your favourite editor. We recommend VS Code.

```sh
code .
```

For each task, close the game, save your changes, rebuild and launch it again:

```sh
cmake --build build/debug/
./build/debug/codesoc-cubed
```

Treat `src/renderer.cpp` and `src/renderer.hpp` as a black box.
You can complete every task without reading or editing them.

Keep your earlier features as you go.
For the final terrain tasks, replace the terrain formula while keeping your respawn key, bedrock, diamonds and day/night cycle.

## 1. Moving Faster

Increase the player's walking speed and check how it affects movement.

**Code Pointer:** Find `walkingSpeed` in `src/world.cpp`.

**Hint:**

Try changing `4.5f` to `6.0f`.
The `f` marks a floating-point number.
`sprintingSpeed` controls how fast you move while holding Shift, so check that sprinting still feels faster than walking.

**Try It:** Walk across a flat area, then sprint along the same route.
Check that the increased speed is still easy to control and that sprinting is faster than walking.

## 2. Low Gravity

Lower gravity and adjust jump speed to change how high the player jumps and how long they stay in the air.

**Code Pointer:** Find `gravity` and `jumpSpeed` in `src/world.cpp`.

**Hint:**

Start by lowering `gravity` from `24.0f`, keeping it positive.
Try half its original value, then adjust `jumpSpeed` from `8.2f`.
Gravity pulls you down, while jump speed gives you the initial upward push.

**Try It:** Turn flight off and jump from the ground.
Compare how high you jump and how long you stay in the air as you change each value.

## 3. Different Planet

Change the colours of the blocks and sky to give the world a different appearance.

**Code Pointers:** Find `blockDefinitions` in `src/blocks.hpp` and `Game::skyColor()` in `src/application.cpp`.

**Hint:**

Block colours use red, green and blue values from 0 to 255.
Grass has a separate top-face colour, so try changing both its side and its top.
The sky colour has a fourth value for opacity: keep that at 255.

**Try It:** Make at least two blocks look different and change the sky.
Place the changed blocks next to each other and check their colours, including their top and side faces.

## 4. Dig Straight Down

Keep mining beneath your feet until you fall through the bottom of the world into the void.
Add a keybind so pressing **R** teleports the player back to spawn.

**Code Pointers:** Look at `Game::updateGame()` in `src/application.cpp`, `World::spawn()` in `src/world.cpp` and `Player` in `src/world.hpp`.

**Hint:**

Find an existing `IsKeyPressed(...)` check and use `KEY_R` for your new action.
`world_.spawn()` gives you the spawn position and `player_.feet` stores your current position.
Reset `player_.verticalVelocity` so you do not bring your falling speed back to spawn.
Also reset `grounded`, turn flight off and clear `jumpRequested_` so movement can settle normally after the teleport.

**Try It:** Fall into the void, press R and check that you return without retaining your falling velocity.
Try the key while walking and flying too.
If you mined away the ground at spawn, restart the game before checking the respawn key.

## 5. Bedrock Bottom

Add a black-grey **Bedrock** block, make it unbreakable and cover the world's bottom layer at `y = 0` with it.

**Code Pointers:** Add the block in `src/blocks.hpp`, choose generated blocks in `World::columnBlock()` in `src/world.cpp` and inspect mining in `Game::updateBlocks()` in `src/application.cpp`.

**Hint:**

Add a definition named `Bedrock` with a dark grey colour.
Once it exists, `blockId("Bedrock")` refers to it.
In `columnBlock()`, handle `y == 0` before the surface and dirt rules.
In the mining code, check `target->block` before replacing the target with `Block::Air`.
Bedrock should remain intact while other blocks remain mineable.

**Try It:** Restart and dig down again.
You should reach a solid bedrock floor, and holding the mine button should leave it intact.
Check that you can still mine dirt and stone.

## 6. Diamond Hunt

Add a light-blue **Diamond** block, put it in the hotbar and make it appear randomly underground like ore in Minecraft.

**Code Pointers:** Use `blockDefinitions` in `src/blocks.hpp`, `palette` in `src/world.hpp` and `World::columnBlock()` in `src/world.cpp`.

**Hint:**

Add a definition named `Diamond`, then replace an empty `Block::Air` slot in `palette` with `blockId("Diamond")`.
Check that you can select and place it before working on generation.
For ore, give some underground stone positions a small chance of becoming diamond.
Keep the surface, dirt and bedrock rules intact.

Ore placement should remain consistent when chunks unload and reload within the same world.
Use the provided `World::randomAt()` helper in `src/world.cpp`.
It returns an integer from 0 to 99 for a block position, with the same result whenever that position is checked in the same world.
For example, `randomAt(x, y, z) < 2` gives roughly a 2% chance of placing a diamond.
Choose where this check belongs in `columnBlock()` so it only replaces underground stone.
You do not need to change the helper or understand its hashing implementation.

**Try It:** Place a diamond from the hotbar, then dig a tunnel to find naturally generated diamonds.
To check that ore generation is repeatable, launch the game with a fixed seed:

```sh
./build/debug/codesoc-cubed --seed 42
```

Dig a tunnel from spawn and record the route to an unmined diamond, including the number of blocks travelled and mined in each direction.
Quit, launch again with the same command and follow the same route to check that the diamond appears in the same place.
Each fresh launch regenerates the terrain and discards your earlier edits.
Simply travelling away and returning after mining does not test regeneration, because edited chunks remain in memory for the session.

## 7. Day and Night

Add a repeating day/night cycle that gradually changes the sky from daytime to night and back again.

**Code Pointer:** Find `Game::skyColor()` in `src/application.cpp`.

**Hint:**

Change `skyColor()` to return a colour that varies over time.
Raylib's `GetTime()` gives elapsed time in seconds.
Turn that time into a repeating cycle, then blend between your daytime sky colour and a dark night colour.
Start with a short cycle so you can watch a full day quickly.
A sine wave can help make the transition smooth.

The application uses the colour returned by `skyColor()` for the background and passes it to the renderer.
The renderer automatically uses that colour for distance fog, so no renderer changes are needed.

**Try It:** Stand somewhere with a clear view and watch a complete day, night and sunrise.
Check for smooth transitions and matching fog at the horizon.

## 8. Superflat

Create a world with a completely flat surface.

**Code Pointer:** Find `World::surfaceHeight()` in `src/world.cpp`.

**Hint:**

Instead of using noise to choose the surface height, return one fixed height for every `x` and `z`.
`baseTerrainHeight` is a useful starting value.
Keep the height between `1` and `height - 2` to leave room for bedrock below and the player above.
Leave `columnBlock()` in charge of the block layers and ore.

**Try It:** Restart, fly over the world and look for bumps.
Build a long wall to check that the ground stays level.
Check that bedrock and diamonds still exist underneath.

## 9. Rolling Hills and Steep Peaks

Generate one world with very shallow hills, then another with very steep hills.

**Code Pointer:** Restore the noise-based calculation in `World::surfaceHeight()` and find `broadTerrainHeight`, `detailTerrainHeight`, `broadTerrainScale` and `detailTerrainScale` in `src/world.cpp`.

**Hint:**

Lower terrain heights make smaller hills.
Try lowering `broadTerrainHeight` from `8.0f` to `5.0f` first.
Larger scale values spread changes over more blocks, making slopes gentler.
For steep hills, try larger heights and smaller scales.
Keep scales positive and preserve the final height clamp.

**Try It:** Inspect each landscape from the ground and the air, looking at several hills to compare their slopes.
Change one value at a time so you can explain what it does.
