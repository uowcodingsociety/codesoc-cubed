#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "world.hpp"

using namespace codesoc;

namespace {

constexpr std::uint32_t testSeed = 42;
constexpr float step = 1.0f / 120.0f;

void check(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void testGeneration() {
    World first(testSeed);
    World second(testSeed);
    World different(testSeed + 1);
    second.streamAround({-120.0f, 40.0f, 80.0f}, 81);
    first.streamAround({80.0f, 40.0f, -120.0f}, 81);
    int seedDifferences = 0;
    for (int z = -130; z <= 130; z += 13) {
        for (int x = -130; x <= 130; x += 13) {
            int surface = first.surfaceHeight(x, z);
            check(surface >= 20 && surface <= 32, "Gentle hills must stay within the chosen range");
            Block top = first.get(x, surface, z);
            check(top == Block::Grass || top == Block::Stone,
                  "Surface must use a supported natural material");
            check(first.get(x, surface - 1, z) == Block::Dirt, "Dirt must sit beneath grass");
            check(first.get(x, surface - 3, z) == Block::Dirt,
                  "Dirt layer must be three blocks deep");
            check(first.get(x, surface - 4, z) == Block::Stone, "Stone must form the core");
            check(first.get(x, 0, z) == Block::Stone, "Base terrain must be stone");
            check(first.get(x, surface + 1, z) == Block::Air, "Sky must be empty");
            check(first.get(x, surface, z) == second.get(x, surface, z),
                  "Chunk loading order must not change generation");
            seedDifferences += surface != different.surfaceHeight(x, z);
        }
    }
    check(seedDifferences > 0, "Different seeds must change the terrain");
    for (std::uint32_t seed : {0u, 1u, std::numeric_limits<std::uint32_t>::max()}) {
        World world(seed);
        check(!world.collides(world.spawn()), "Spawn must be safe at extreme seeds");
    }
    check(first.get(0, -1, 0) == Block::Air && first.get(0, height, 0) == Block::Air,
          "Outside the vertical terrain range must be empty");
}

void testStreamingAndEdits() {
    check(World::chunkFor(-1, -1) == ChunkCoord(-1, -1), "Negative cells need floor division");
    check(World::chunkFor(-16, -17) == ChunkCoord(-1, -2), "Negative seams need exact chunks");
    check(World::chunkFor(16, 15) == ChunkCoord(1, 0), "Positive seams need exact chunks");

    World world(testSeed);
    world.preloadSpawn();
    check(world.loadedCount() == 9, "Nine spawn chunks must be preloaded");
    world.streamAround(world.spawn());
    check(world.loadedCount() == 11, "Streaming must generate at most two chunks per frame");
    world.streamAround(world.spawn(), 81);
    check(world.loadedCount() == 81, "Four-chunk loading radius must fill a 9x9 area");
    check(world.set(0, 0, 0, Block::Air), "Bottom stone must be mineable");
    check(!world.set(0, -1, 0, Block::Stone), "Blocks cannot be placed below the terrain range");
    check(!world.set(0, height, 0, Block::Stone), "Blocks cannot be placed above the range");
    check(!world.set(0, 0, 0, Block::Count), "Invalid materials must be rejected");

    world.streamAround({320.5f, 40.0f, 320.5f}, 81);
    check(world.loadedCount() == 81, "Distant travel must unload old active chunks");
    check(world.retainedCount() == 82, "Only the edited old chunk should remain in memory");
    check(world.get(0, 0, 0) == Block::Air, "An edit must survive unloading");
    check(world.get(16, 0, 0) == Block::Stone, "Unedited unloaded chunks must regenerate");
    world.streamAround(world.spawn(), 81);
    check(world.get(0, 0, 0) == Block::Air, "An edit must survive returning to the chunk");

    world.clearDirty({-1, 0});
    world.clearDirty({0, 0});
    check(world.set(-1, 50, 0, Block::Stone), "Negative seam edit must succeed");
    check(world.isDirty({-1, 0}) && world.isDirty({0, 0}),
          "Editing at a seam must invalidate both meshes");
}

void testPhysicsAndTargeting() {
    World world(testSeed);
    Player player;
    player.feet = world.spawn();
    for (int i = 0; i < 120; ++i) {
        world.move(player, {}, step);
    }
    check(player.grounded, "Player must settle on the terrain");
    float ground = player.feet.y;
    world.move(player, {0, 0, 0, true, false}, step);
    check(player.feet.y > ground, "Space jump must lift the player");

    check(world.set(17, 50, 0, Block::Stone), "A block can be placed across a chunk seam");
    auto hit = world.raycast({15.5f, 50.5f, 0.5f}, {1, 0, 0});
    check(hit && hit->cell == Cell(17, 50, 0) && hit->previous == Cell(16, 50, 0),
          "Raycasts must cross chunk seams");
    check(!world.raycast({15.5f, 50.5f, 0.5f}, {1, 0, 0}, 1.0f), "Raycasts must respect reach");
    check(!world.raycast({15.5f, 50.5f, 0.5f}, {0, 0, 0}), "Zero-length raycasts must terminate");
    check(world.overlapsPlayer({0, int(ground) + 1, 0}, player.feet),
          "Placement must detect player overlap");

    check(world.set(16, 50, 0, Block::Stone), "A seam wall can be built");
    Player wallTraveller;
    wallTraveller.feet = {15.5f, 50.0f, 0.5f};
    wallTraveller.yaw = 1.5707963f;
    wallTraveller.flying = true;
    world.move(wallTraveller, {1, 0, 0, false, true}, 0.1f);
    check(wallTraveller.feet.x < 15.71f, "Collision must stop movement across chunk seams");

    Player traveller;
    traveller.feet = {127.5f, 50.0f, 0.5f};
    traveller.yaw = 1.5707963f;
    traveller.flying = true;
    world.move(traveller, {1, 0, 0, false, true}, 0.1f);
    check(traveller.feet.x > 128.0f, "Flight must cross the old positive world edge");
    traveller.feet = {-127.5f, 50.0f, 0.5f};
    traveller.yaw = -1.5707963f;
    world.move(traveller, {1, 0, 0, false, true}, 0.1f);
    check(traveller.feet.x < -128.0f, "Flight must cross the old negative world edge");

    int surface = world.surfaceHeight(0, 0);
    for (int y = 0; y <= surface; ++y) {
        world.set(0, y, 0, Block::Air);
    }
    Player falling;
    falling.feet = {0.5f, float(surface) + 1.01f, 0.5f};
    for (int i = 0; i < 500; ++i) {
        world.move(falling, {}, step);
    }
    check(falling.feet.y < -1.0f, "Player must be able to fall into the void");
    float bottom = falling.feet.y;
    falling.flying = true;
    world.move(falling, {0, 0, 1, false, false}, 0.1f);
    check(falling.feet.y > bottom, "Flight must rescue a falling player");
}

} // namespace

int main() {
    try {
        testGeneration();
        testStreamingAndEdits();
        testPhysicsAndTargeting();
        std::cout
            << "PASS: deterministic hills, chunks, edits, seams, collision, targeting and void\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
