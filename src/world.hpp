#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace codesoc {

constexpr int height = 64;
constexpr int chunkSize = 16;

/** Chunk radii are measured horizontally. The unload buffer avoids churn near the load edge. */
constexpr int loadRadius = 4;
constexpr int unloadRadius = 5;
constexpr int chunkVolume = chunkSize * chunkSize * height;

enum class Block : std::uint8_t { Air, Grass, Dirt, Stone, Count };

constexpr std::array<Block, 9> palette = {Block::Grass,
                                          Block::Dirt,
                                          Block::Stone,
                                          Block::Air,
                                          Block::Air,
                                          Block::Air,
                                          Block::Air,
                                          Block::Air,
                                          Block::Air};

const char* blockName(Block block);
bool solid(Block block);

struct Vec3 {
    float x = 0;
    float y = 0;
    float z = 0;
};

struct Cell {
    int x = 0;
    int y = 0;
    int z = 0;

    bool operator==(const Cell&) const = default;
};

struct ChunkCoord {
    int x = 0;
    int z = 0;

    bool operator==(const ChunkCoord&) const = default;
};

struct ChunkCoordHash {
    std::size_t operator()(ChunkCoord coord) const noexcept;
};

/** A ray hit and the cell immediately before it, used as the placement target. */
struct Hit {
    Cell cell;
    Cell previous;
    Block block;
    float distance;
};

struct Player {
    /** Position is measured at the player's feet in world blocks. Y increases upward. */
    Vec3 feet = {0.5f, 36.0f, 0.5f};

    /** Look angles are in radians. */
    float yaw = 0.7f;
    float pitch = -0.15f;
    float verticalVelocity = 0;
    bool flying = false;
    bool grounded = false;

    /** Index into palette, including its empty slots. */
    int selected = 0;

    Vec3 eye() const;
    Vec3 direction() const;
};

struct Input {
    /** Forward and right follow the player's yaw; vertical follows world Y.
     *  Each axis normally ranges from -1 to 1.
     */
    float forward = 0;
    float right = 0;
    float vertical = 0;
    bool jump = false;
    bool sprint = false;
};

/** Owns generated terrain, session edits, collision and player movement.
 *  Unchanged chunks can unload and regenerate; edited chunks remain in memory for this run.
 */
class World {
public:
    explicit World(std::uint32_t seed);

    /** Starts a fresh world, discarding all loaded chunks and session edits. */
    void generate(std::uint32_t seed);
    void preloadSpawn();

    /** Loads nearby chunks and unloads distant ones.
     *  The limit counts newly generated chunks, not edited chunks restored to the active area.
     */
    void streamAround(Vec3 position, int maximumNewChunks = 2);

    /** Uses floor division so negative block coordinates map to the correct chunk. */
    static ChunkCoord chunkFor(int x, int z);
    static bool contains(int y);

    /** Reads edits when present or generates the natural block without loading a chunk.
     *  Coordinates outside the vertical world range read as air.
     */
    Block get(int x, int y, int z) const;

    /** Stores a session edit and marks affected meshes for rebuilding.
     *  @return False for an invalid height or block type, or when the value is unchanged.
     */
    bool set(int x, int y, int z, Block block);

    /** Returns the seed-based natural surface height, regardless of edits or chunk loading. */
    int surfaceHeight(int x, int z) const;

    /** Tests the player's collision box at a feet position. There is no floor below the world. */
    bool collides(Vec3 feet) const;
    bool overlapsPlayer(Cell cell, Vec3 feet) const;

    /** Finds the first solid block within reach, measured in world blocks.
     *  Direction is normalized internally, so its length does not affect reach.
     *  Hit::previous is the adjacent cell before the hit, or the hit cell if the origin is solid.
     */
    std::optional<Hit> raycast(Vec3 origin, Vec3 direction, float reach = 7.0f) const;
    Vec3 spawn() const;

    /** Advances movement for dt seconds with collision, gravity and flight.
     *  Clamps dt to 0.1 seconds to limit large simulation steps.
     */
    void move(Player& player, Input input, float dt) const;

    std::uint32_t seed() const {
        return seed_;
    }

    std::size_t loadedCount() const {
        return active_.size();
    }

    std::size_t retainedCount() const {
        return chunks_.size();
    }

    const std::unordered_set<ChunkCoord, ChunkCoordHash>& activeChunks() const {
        return active_;
    }

    bool isDirty(ChunkCoord coord) const;
    void clearDirty(ChunkCoord coord);

private:
    struct ChunkData {
        std::array<Block, chunkVolume> blocks = {};
        bool modified = false;
    };

    static std::size_t index(int localX, int y, int localZ);

    /** Selects the natural top block for a column before any session edits. */
    Block surfaceBlock(int x, int z) const;
    Block columnBlock(int x, int y, int z, int surface) const;
    Block generatedBlock(int x, int y, int z) const;
    ChunkData& ensureChunk(ChunkCoord coord);
    void markAffectedChunksDirty(int x, int z);
    void moveAlongAxis(Player& player, Vec3 displacement) const;

    std::uint32_t seed_ = 0;

    /** Active chunks are loaded for rendering; modified chunks remain stored after deactivation. */
    std::unordered_map<ChunkCoord, ChunkData, ChunkCoordHash> chunks_;
    std::unordered_set<ChunkCoord, ChunkCoordHash> active_;

    /** Includes chunks whose visible faces may have changed, including neighbours of edge edits. */
    std::unordered_set<ChunkCoord, ChunkCoordHash> dirty_;
};

} // namespace codesoc
