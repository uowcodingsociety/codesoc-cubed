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
constexpr int loadRadius = 4;
constexpr int unloadRadius = 5;
constexpr int chunkVolume = chunkSize * chunkSize * height;

enum class Block : std::uint8_t { Air, Grass, Dirt, Stone, Count };
constexpr std::array<Block, 9> palette{Block::Grass,
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
    float x{}, y{}, z{};
};

struct Cell {
    int x{}, y{}, z{};
    bool operator==(const Cell&) const = default;
};

struct ChunkCoord {
    int x{}, z{};
    bool operator==(const ChunkCoord&) const = default;
};

struct ChunkCoordHash {
    std::size_t operator()(ChunkCoord coord) const noexcept;
};

struct Hit {
    Cell cell;
    Cell previous;
    Block block;
    float distance;
};

struct Player {
    Vec3 feet{0.5f, 36.0f, 0.5f};
    float yaw{0.7f}, pitch{-0.15f}, verticalVelocity{};
    bool flying{}, grounded{};
    int selected{};

    Vec3 eye() const;
    Vec3 direction() const;
};

struct Input {
    float forward{}, right{}, vertical{};
    bool jump{}, sprint{};
};

/** Generated chunks unload as the player moves; changed chunks remain in memory for this run. */
class World {
public:
    explicit World(std::uint32_t seed);
    void generate(std::uint32_t seed);
    void preloadSpawn();
    void streamAround(Vec3 position, int maximumNewChunks = 2);

    static ChunkCoord chunkFor(int x, int z);
    static bool contains(int y);
    Block get(int x, int y, int z) const;
    bool set(int x, int y, int z, Block block);
    int surfaceHeight(int x, int z) const;
    bool collides(Vec3 feet) const;
    bool overlapsPlayer(Cell cell, Vec3 feet) const;
    std::optional<Hit> raycast(Vec3 origin, Vec3 direction, float reach = 7.0f) const;
    Vec3 spawn() const;
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
        std::array<Block, chunkVolume> blocks{};
        bool modified{};
    };

    static std::size_t index(int localX, int y, int localZ);
    Block surfaceBlock(int x, int z) const;
    Block columnBlock(int x, int y, int z, int surface) const;
    Block generatedBlock(int x, int y, int z) const;
    ChunkData& ensureChunk(ChunkCoord coord);
    void markAffectedChunksDirty(int x, int z);
    void moveAlongAxis(Player& player, Vec3 displacement) const;

    std::uint32_t seed_{};
    std::unordered_map<ChunkCoord, ChunkData, ChunkCoordHash> chunks_;
    std::unordered_set<ChunkCoord, ChunkCoordHash> active_;
    std::unordered_set<ChunkCoord, ChunkCoordHash> dirty_;
};

} // namespace codesoc
