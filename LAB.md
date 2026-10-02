# Build Your Own Block World

Your world, your rules!
Start by exploring the game, then turn it into a speedy, low-gravity playground with hidden diamonds and strange landscapes.
Work through the missions in order and get as far as you can.
Each change is an experiment: make a prediction, try it in the game and see what surprises you.

## Get Started

Open a terminal and run these commands.

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

5. Launch your first world:

```sh
./build/release/codesoc-cubed --seed 42
```

Take a look around before changing anything.
Using the same seed lets you compare your changes on the same starting terrain.

- Move with WASD and look around with the mouse
- Press Space to jump, double-tap Space to toggle flight and hold Shift to move faster
- Hold Ctrl to descend while flying
- Mine with the left mouse button and place blocks with the right mouse button
- Select grass, dirt or stone with keys 1–3
- Press F3 to show or hide the FPS counter and world seed
- Press Escape or Q to quit

## Get Ready to Tinker

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
./build/debug/codesoc-cubed --seed 42
```

4. Open the code in your favourite editor. We recommend VS Code.

```sh
code .
```

For each mission, close the game, save your changes, rebuild and launch it again:

```sh
cmake --build build/debug/
./build/debug/codesoc-cubed --seed 42
```

If the build fails, start with the first error message and check the file and line it mentions.
Keep your earlier features as you go.
For the final terrain missions, swap the landscape formula while keeping your respawn key, bedrock, diamonds and day/night cycle.

## 1. Moving Faster

Give your player a speed boost!
Change their walking speed, then find a pace that feels fun to control.

**Code Pointer:** Find `walkingSpeed` in `src/world.cpp`.

<details>
<summary>Hint: Start with a small boost</summary>

Try changing `4.5f` to `6.0f`.
The `f` marks a floating-point number.
`sprintingSpeed` controls how fast you move while holding Shift, so check that sprinting still feels faster than walking.

</details>

**Try It:** Walk between two landmarks before and after your change, then sprint the same route.
Can you make a speedy player who still stops where you want?

## 2. Low Gravity

One small jump for your player, one giant leap across the hillside.
Change gravity and jump speed to make the world feel like a low-gravity moon.

**Code Pointer:** Find `gravity` and `jumpSpeed` in `src/world.cpp`.

<details>
<summary>Hint: Change one value at a time</summary>

Start by lowering `gravity` from `24.0f`, keeping it positive.
Try half its original value, then adjust `jumpSpeed` from `8.2f`.
Gravity pulls you down, while jump speed gives you the initial upward push.

</details>

**Try It:** Turn flight off and jump from the ground.
Compare how high you jump and how long you stay in the air as you change each value.

## 3. Different Planet

Who says grass has to be green?
Give the blocks and sky a new colour palette for an alien planet.

**Code Pointers:** Find `blockDefinitions` in `src/blocks.hpp` and `Renderer::sky()` in `src/renderer.cpp`.

<details>
<summary>Hint: Mix your own colours</summary>

Block colours use red, green and blue values from 0 to 255.
Grass has a separate top-face colour, so try changing both its side and its top.
The sky colour has a fourth value for opacity: keep that at 255.

</details>

**Try It:** Make at least two blocks look different and change the sky.
Build a small sculpture to show off your planet's colours.

## 4. Dig Straight Down

Time to break the usual Minecraft rule: dig straight down!
Keep mining beneath your feet until you fall through the bottom of the world into the void.
Then give yourself a rescue button: pressing **R** should teleport you back to spawn.

**Code Pointers:** Look at `Game::updateGame()` in `src/application.cpp`, `World::spawn()` in `src/world.cpp` and `Player` in `src/world.hpp`.

<details>
<summary>Hint: Follow an existing keybind</summary>

Find an existing `IsKeyPressed(...)` check and use `KEY_R` for your new action.
`world_.spawn()` gives you the spawn position and `player_.feet` stores your current position.
Reset `player_.verticalVelocity` so you do not bring your falling speed back to spawn.
Also reset `grounded`, turn flight off and clear `jumpRequested_` so movement can settle normally after the teleport.

</details>

**Try It:** Fall into the void, press R and check that you return without plummeting again.
Try the key while walking and flying too.
If you mined away the ground at spawn, restart the game before checking your rescue button.

## 5. Bedrock Bottom

Install a floor that even the most determined miner cannot break.
Add a black-grey **Bedrock** block, make it unbreakable and cover the world's bottom layer at `y = 0` with it.

**Code Pointers:** Add the block in `src/blocks.hpp`, choose generated blocks in `World::columnBlock()` in `src/world.cpp` and inspect mining in `Game::updateBlocks()` in `src/application.cpp`.

<details>
<summary>Hint: A new block needs a look and a rule</summary>

Add a definition named `Bedrock` with a dark grey colour.
Once it exists, `blockId("Bedrock")` refers to it.
In `columnBlock()`, handle `y == 0` before the surface and dirt rules.
In the mining code, check `target->block` before replacing the target with `Block::Air`.
Bedrock should stay put while other blocks remain mineable.

</details>

**Try It:** Restart and dig down again.
You should reach a solid bedrock floor, and holding the mine button should leave it intact.
Check that you can still mine dirt and stone.

## 6. Diamond Hunt

Hide some treasure beneath your planet's surface.
Add a light-blue **Diamond** block, put it in the hotbar and make it appear randomly underground like ore in Minecraft.

**Code Pointers:** Use `blockDefinitions` in `src/blocks.hpp`, `palette` in `src/world.hpp` and `World::columnBlock()` in `src/world.cpp`.

<details>
<summary>Hint: Start with a block you can place</summary>

Add a definition named `Diamond`, then replace an empty `Block::Air` slot in `palette` with `blockId("Diamond")`.
Check that you can select and place it before working on generation.
For ore, give some underground stone positions a small chance of becoming diamond.
Keep the surface, dirt and bedrock rules intact.

The same seed and coordinates should always produce the same result.
Look at `coordinateHash()` in `src/world.cpp` for a starting point and include depth (`y`) as well as `x`, `z` and the seed in your ore decision.
Avoid a fresh random roll every time a block or chunk is generated.

</details>

**Try It:** Place a diamond from the hotbar, then dig a tunnel to find naturally generated diamonds.
Restart with the same seed and check that the ore returns in the same places.
Try another seed for a different treasure hunt.

## 7. Day and Night

Your planet needs a bedtime.
Add a repeating day/night cycle that gradually changes the sky from daytime to night and back again.

**Code Pointers:** Start with `Renderer::sky()` and `Renderer::draw()` in `src/renderer.cpp`, then find `ClearBackground(Renderer::sky())` in `src/application.cpp`.

<details>
<summary>Hint: Turn time into a colour</summary>

Raylib's `GetTime()` gives elapsed time in seconds.
Turn that time into a repeating cycle, then blend between your daytime sky colour and a dark night colour.
Start with a short cycle so you can watch a full day quickly.
A sine wave can help make the transition smooth.

The background and distance fog both use `Renderer::sky()`.
Keep them on the same cycle so distant terrain matches the sky.

</details>

**Try It:** Stand somewhere with a clear view and watch a complete day, night and sunrise.
Check for smooth transitions and matching fog at the horizon.

## 8. Superflat

Flatten the planet into the ultimate building plot.
Create a world with a completely flat surface.

**Code Pointer:** Find `World::surfaceHeight()` in `src/world.cpp`.

<details>
<summary>Hint: Every column needs the same height</summary>

Instead of using noise to choose the surface height, return one fixed height for every `x` and `z`.
`baseTerrainHeight` is a useful starting value.
Keep the height between `1` and `height - 2` to leave room for bedrock below and the player above.
Leave `columnBlock()` in charge of the block layers and ore.

</details>

**Try It:** Restart, fly over the world and look for bumps.
Build a long wall or runway on your new flat ground.
Check that bedrock and diamonds still exist underneath.

## 9. Rolling Hills and Steep Peaks

Trade your runway for a landscape rollercoaster.
Generate one world with very shallow hills, then another with very steep hills.

**Code Pointer:** Restore the noise-based calculation in `World::surfaceHeight()` and find `broadTerrainHeight`, `detailTerrainHeight`, `broadTerrainScale` and `detailTerrainScale` in `src/world.cpp`.

<details>
<summary>Hint: Height and width work together</summary>

Lower terrain heights make smaller hills.
Try lowering `broadTerrainHeight` from `8.0f` to `5.0f` first.
Larger scale values spread changes over more blocks, making slopes gentler.
For steep hills, try larger heights and smaller scales.
Keep scales positive and preserve the final height clamp.

</details>

**Try It:** Use the same seed for both landscapes and compare the slopes from the ground and the air.
Change one value at a time so you can explain what it does.

## 10. Wave Worlds

Give your terrain a rhythm.
Replace the noise-based surface calculation with each of these functions in turn: **sine wave**, **saw wave** and **square wave**.
Try all three as separate worlds.

**Code Pointer:** Change the height calculation in `World::surfaceHeight()` in `src/world.cpp`.

<details>
<summary>Hint: Start with waves along one direction</summary>

Use `x` as the position along your wave and leave `z` out at first.
Choose a base height, an amplitude (how tall the wave is) and a wavelength (how many blocks before it repeats).
Keep your final integer height clamped between `1` and `height - 2`.

- **Sine wave:** `std::sin()` produces smooth repeating rises and falls
- **Saw wave:** Use the fractional part of `float(x) / wavelength` to make a ramp that rises and suddenly drops, using `std::floor()` so it also repeats correctly at negative coordinates
- **Square wave:** Split each repeating period into a high half and a low half to make alternating raised and lowered platforms

</details>

**Try It:** Fly along the wave direction to spot the pattern, then explore on foot.
Check both positive and negative coordinates and make sure the pattern continues across chunk boundaries.
Which world would make the best obstacle course?
