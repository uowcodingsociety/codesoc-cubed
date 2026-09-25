# Codesoc Cubed Starter Specification

This document describes the project as shipped to a Welcome Week participant, before they make the changes in [LAB.md](LAB.md).
It is the shared product and technical contract for users, developers and coding agents.
The lab deliberately changes some starter values and may add features, so a participant's completed exercise is not required to match every baseline detail below.
For build instructions and platform setup, see [README.md](README.md).

## Purpose and Scope

Codesoc Cubed is a small, single-player C++ voxel sandbox designed for a solo 90-minute CodeSoc Welcome Week lab at the University of Warwick.
A participant should be able to build it, enter a world immediately, explore and make a bounded change to the terrain generator without learning a large game architecture first.
There is no account, network play, submission system or persistent progression.

The shipped executable and window title are `codesoc-cubed` and “Codesoc Cubed” respectively.
The C++ namespace is `codesoc`.
Only the current project branding should appear in tracked project text and code.

## Launch and Session

- A normal launch opens a visible, resizable game window and places the player directly in a generated world, without a title screen or menu
- The initial window size is 1440 × 900, with a minimum of 960 × 640
- Each session starts with a new 32-bit world seed, unless `--seed NUMBER` supplies one for reproducibility
- `--help` prints the supported command-line options, and invalid options fail with an error
- Escape or Q quits immediately, without a confirmation or save step
- `--smoke-test` is a developer-only, hidden-window check that runs for 60 frames and exits

World changes exist only for the current process.
The game does not save or load worlds.
Raylib's built-in F12 shortcut saves a screenshot of the game window.

## World Model

A world is identified by its seed and integer block coordinates `(x, y, z)`.
X and Z have no fixed gameplay border, including across positive and negative chunk coordinates.
The generated vertical range is `y = 0..63`, and queries outside it return air.
The area below the bottom layer is void rather than an invisible floor.

The starter generator produces deterministic, gently rolling hills.
For a given seed and X/Z coordinate, the natural surface height is 20–32 inclusive and does not depend on chunk loading order.
Each natural column has grass on top, three dirt blocks immediately below and stone beneath to `y = 0`.
The starter has no natural stone outcrops: the surface-material helper returns grass until a participant changes it in Stage 2 of the lab.
There are no caves, rivers, trees, water or biome systems in the starter.

The four usable block types are air, grass, dirt and stone.
Air is empty, while grass, dirt and stone are solid and can be mined, including the bottom stone layer.
World edits are allowed only within the vertical range.

## Chunk Streaming and Edits

A chunk is a 16 × 16 horizontal area spanning all 64 terrain levels.
The player starts with a 3 × 3 area of chunks generated around the spawn point.
As the player travels, the game works toward a square loading radius of four chunks, generating at most two new chunks per frame during normal streaming.
Active chunks remain until they are more than five chunks from the player, providing a one-chunk unload buffer.

Unedited distant chunks are discarded from memory and reproduce the same terrain when queried or loaded again.
Edited chunks remain in memory for the rest of the session, even after leaving the active area, so mining and placement survive travel away and back.
Edits on a chunk edge invalidate the neighbouring chunk mesh as well as the edited chunk mesh.
No chunk data is written to disk.

## Player and Interaction

The player spawns safely above the surface near `(0, 0)` and views the world from a first-person camera.
Movement and collision work across chunk boundaries, with no former fixed-world edge.
Walking obeys gravity and solid-block collision.
There is no automatic respawn if the player falls into the void, though flight can be toggled during a fall.

| Input | Starter Behaviour |
| --- | --- |
| WASD and mouse | Move and look |
| Space | Jump when grounded, or rise while flying |
| Double Space within 250 ms | Toggle flight |
| Left Shift | Move faster |
| Left Ctrl | Descend while flying |
| Left mouse button | Mine the targeted block |
| Right mouse button | Place the selected block beside the targeted block |
| Middle mouse button | Select a targeted block type if it is in the palette |
| 1–9 or mouse wheel | Select a hotbar slot |
| F3 | Toggle position, seed, chunk, loaded-chunk, FPS and triangle information |
| H | Toggle the controls card |
| F11 | Toggle borderless fullscreen |
| F12 | Save a screenshot using Raylib's built-in shortcut |
| Escape or Q | Quit |

Mining and placement use a seven-block targeting reach.
Placement requires an empty cell, a non-empty selected slot and no overlap with the player.
The first three hotbar slots contain grass, dirt and stone.
The remaining six slots are selectable but empty and cannot place a block.
There is no inventory limit or survival system.

## Presentation

The renderer draws exposed block faces as chunk meshes and rebuilds meshes when relevant chunks change.
Grass has a green top and dirt-coloured sides, while dirt and stone use distinct flat colours.
The starter has a fixed sky colour and distance fog, with no day-night cycle or moving sky objects.
The HUD provides a crosshair, targeted-block outline, selected-block name, nine-slot hotbar, controls card and optional debug information.

## Build and Verification Contract

The project uses C++20, CMake 3.25 or newer and Raylib 6.0.
CMake fetches the pinned Raylib release and builds the `codesoc-cubed` executable plus graphics-independent world tests.
The project does not choose a default Release build type: a single-config generator such as Ninja uses the configuration supplied at CMake configure time.
Linux is the verified lab platform, while the README gives best-effort macOS and Windows instructions.

A change to the starter is ready only when it builds without compiler errors, all tests pass, formatting and lint checks pass, and normal launch produces a visible window.
The hidden smoke test checks startup, rendering, movement, flight and editing but does not prove desktop visibility.

## Lab Boundary and Document Roles

[LAB.md](LAB.md) defines the participant activity: Stage 0 explores the starter, Stage 1 changes movement and hill constants, Stage 2 adds deterministic stone outcrops and Stage 3 is optional open-ended work.
Those exercises are intended to alter the starter baseline without changing the requirement that generation remain deterministic across chunk boundaries and reloads.
There is no required submission.

This specification defines the shipped behaviour and constraints.
[README.md](README.md) explains how to build and use it, while [LAB.md](LAB.md) instructs participants what to change.
When the shipped starter intentionally changes, update this specification, the relevant tests and the user-facing instructions together.
