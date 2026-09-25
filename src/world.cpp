#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

#include "world.hpp"

namespace codesoc {
namespace {

constexpr std::uint32_t hashX = 374761393u;
constexpr std::uint32_t hashZ = 2246822519u;
constexpr std::uint32_t hashMix = 1274126177u;
constexpr float broadTerrainScale = 49.0f;
constexpr float detailTerrainScale = 17.0f;
constexpr std::uint32_t detailSeedOffset = 41;
constexpr int baseTerrainHeight = 20;
constexpr float broadTerrainHeight = 8.0f;
constexpr float detailTerrainHeight = 4.0f;
constexpr int dirtDepth = 3;

constexpr float playerRadius = 0.3f;
constexpr float playerHeight = 1.8f;
constexpr float playerEyeHeight = 1.62f;
constexpr float collisionEpsilon = 0.001f;
constexpr float maximumFrameTime = 0.1f;
constexpr float maximumMovementStep = 0.12f;
constexpr int collisionSearchIterations = 10;
constexpr float walkingSpeed = 4.5f;
constexpr float sprintingSpeed = 7.0f;
constexpr float flyingSpeed = 9.0f;
constexpr float fastFlyingSpeed = 18.0f;
constexpr float jumpSpeed = 8.2f;
constexpr float gravity = 24.0f;
constexpr float terminalVelocity = -40.0f;
constexpr float directionEpsilon = 0.00001f;

std::uint32_t coordinateHash(int x, int z, std::uint32_t seed) {
    auto hash = seed ^ (std::uint32_t(x) * hashX) ^ (std::uint32_t(z) * hashZ);
    hash = (hash ^ (hash >> 13)) * hashMix;
    return hash ^ (hash >> 16);
}

float smoothValueNoise(float x, float z, std::uint32_t seed) {
    const int ix = int(std::floor(x));
    const int iz = int(std::floor(z));
    float u = x - float(ix);
    float v = z - float(iz);
    u = u * u * (3 - 2 * u);
    v = v * v * (3 - 2 * v);
    auto value = [&](int a, int b) {
        constexpr std::uint32_t valueMask = 65535;
        return float(coordinateHash(a, b, seed) & valueMask) / float(valueMask);
    };

    return std::lerp(std::lerp(value(ix, iz), value(ix + 1, iz), u),
                     std::lerp(value(ix, iz + 1), value(ix + 1, iz + 1), u),
                     v);
}

int floorDiv(int value, int divisor) {
    int quotient = value / divisor;
    if (value < 0 && value % divisor != 0) {
        --quotient;
    }
    return quotient;
}

int distance(ChunkCoord a, ChunkCoord b) {
    return std::max(std::abs(a.x - b.x), std::abs(a.z - b.z));
}

} // namespace

std::size_t ChunkCoordHash::operator()(ChunkCoord coord) const noexcept {
    return std::size_t(coordinateHash(coord.x, coord.z, 0x51ed270bu));
}

const char* blockName(Block block) {
    constexpr std::array names{"Empty", "Grass", "Dirt", "Stone"};
    auto value = std::size_t(block);
    return value < names.size() ? names[value] : "Unknown";
}

bool solid(Block block) {
    return block != Block::Air;
}

Vec3 Player::eye() const {
    return {feet.x, feet.y + playerEyeHeight, feet.z};
}

Vec3 Player::direction() const {
    return {std::sin(yaw) * std::cos(pitch), std::sin(pitch), std::cos(yaw) * std::cos(pitch)};
}

World::World(std::uint32_t seed) {
    generate(seed);
}

void World::generate(std::uint32_t seed) {
    seed_ = seed;
    chunks_.clear();
    active_.clear();
    dirty_.clear();
}

ChunkCoord World::chunkFor(int x, int z) {
    return {floorDiv(x, chunkSize), floorDiv(z, chunkSize)};
}

bool World::contains(int y) {
    return y >= 0 && y < height;
}

std::size_t World::index(int localX, int y, int localZ) {
    return std::size_t(localX + chunkSize * (localZ + chunkSize * y));
}

int World::surfaceHeight(int x, int z) const {
    float broad =
        smoothValueNoise(float(x) / broadTerrainScale, float(z) / broadTerrainScale, seed_);
    float detail = smoothValueNoise(float(x) / detailTerrainScale,
                                    float(z) / detailTerrainScale,
                                    seed_ + detailSeedOffset);
    return std::clamp(baseTerrainHeight + int(std::round(broad * broadTerrainHeight +
                                                         detail * detailTerrainHeight)),
                      1,
                      height - 2);
}

Block World::surfaceBlock(int x, int z) const {
    static_cast<void>(x);
    static_cast<void>(z);
    return Block::Grass;
}

Block World::columnBlock(int x, int y, int z, int surface) const {
    if (y > surface) {
        return Block::Air;
    }
    if (y == surface) {
        return surfaceBlock(x, z);
    }
    return y >= surface - dirtDepth ? Block::Dirt : Block::Stone;
}

Block World::generatedBlock(int x, int y, int z) const {
    return contains(y) ? columnBlock(x, y, z, surfaceHeight(x, z)) : Block::Air;
}

World::ChunkData& World::ensureChunk(ChunkCoord coord) {
    auto [entry, inserted] = chunks_.try_emplace(coord);
    if (!inserted) {
        return entry->second;
    }

    for (int localZ = 0; localZ < chunkSize; ++localZ) {
        for (int localX = 0; localX < chunkSize; ++localX) {
            int x = coord.x * chunkSize + localX;
            int z = coord.z * chunkSize + localZ;
            int surface = surfaceHeight(x, z);
            for (int y = 0; y < height; ++y) {
                entry->second.blocks[index(localX, y, localZ)] = columnBlock(x, y, z, surface);
            }
        }
    }

    dirty_.insert(coord);
    return entry->second;
}

void World::preloadSpawn() {
    for (int z = -1; z <= 1; ++z) {
        for (int x = -1; x <= 1; ++x) {
            ChunkCoord coord{x, z};
            ensureChunk(coord);
            active_.insert(coord);
        }
    }
}

void World::streamAround(Vec3 position, int maximumNewChunks) {
    ChunkCoord centre = chunkFor(int(std::floor(position.x)), int(std::floor(position.z)));
    for (auto it = active_.begin(); it != active_.end();) {
        if (distance(*it, centre) <= unloadRadius) {
            ++it;
            continue;
        }
        auto chunk = chunks_.find(*it);
        if (chunk != chunks_.end() && !chunk->second.modified) {
            chunks_.erase(chunk);
        }
        dirty_.erase(*it);
        it = active_.erase(it);
    }

    std::vector<ChunkCoord> missing;
    for (int z = -loadRadius; z <= loadRadius; ++z) {
        for (int x = -loadRadius; x <= loadRadius; ++x) {
            ChunkCoord coord{centre.x + x, centre.z + z};
            if (!active_.contains(coord)) {
                missing.push_back(coord);
            }
        }
    }

    std::sort(missing.begin(), missing.end(), [&](ChunkCoord a, ChunkCoord b) {
        int first = distance(a, centre);
        int second = distance(b, centre);
        if (first != second) {
            return first < second;
        }
        return a.z != b.z ? a.z < b.z : a.x < b.x;
    });

    int generated = 0;
    for (ChunkCoord coord : missing) {
        bool alreadyRetained = chunks_.contains(coord);
        if (!alreadyRetained && generated >= maximumNewChunks) {
            continue;
        }
        ensureChunk(coord);
        active_.insert(coord);
        dirty_.insert(coord);
        if (!alreadyRetained) {
            ++generated;
        }
    }
}

Block World::get(int x, int y, int z) const {
    if (!contains(y)) {
        return Block::Air;
    }
    ChunkCoord coord = chunkFor(x, z);
    auto chunk = chunks_.find(coord);
    if (chunk == chunks_.end()) {
        return generatedBlock(x, y, z);
    }

    return chunk->second.blocks[index(x - coord.x * chunkSize, y, z - coord.z * chunkSize)];
}

bool World::set(int x, int y, int z, Block block) {
    if (!contains(y) || block >= Block::Count || get(x, y, z) == block) {
        return false;
    }

    ChunkCoord coord = chunkFor(x, z);
    auto& chunk = ensureChunk(coord);
    chunk.blocks[index(x - coord.x * chunkSize, y, z - coord.z * chunkSize)] = block;
    chunk.modified = true;
    markAffectedChunksDirty(x, z);
    return true;
}

void World::markAffectedChunksDirty(int x, int z) {
    ChunkCoord coord = chunkFor(x, z);
    dirty_.insert(coord);
    int localX = x - coord.x * chunkSize;
    int localZ = z - coord.z * chunkSize;
    if (localX == 0) {
        dirty_.insert({coord.x - 1, coord.z});
    }
    if (localX == chunkSize - 1) {
        dirty_.insert({coord.x + 1, coord.z});
    }
    if (localZ == 0) {
        dirty_.insert({coord.x, coord.z - 1});
    }
    if (localZ == chunkSize - 1) {
        dirty_.insert({coord.x, coord.z + 1});
    }
}

bool World::isDirty(ChunkCoord coord) const {
    return dirty_.contains(coord);
}

void World::clearDirty(ChunkCoord coord) {
    dirty_.erase(coord);
}

bool World::overlapsPlayer(Cell cell, Vec3 feet) const {
    return feet.x + playerRadius > cell.x && feet.x - playerRadius < cell.x + 1 &&
           feet.y + playerHeight > cell.y && feet.y < cell.y + 1 &&
           feet.z + playerRadius > cell.z && feet.z - playerRadius < cell.z + 1;
}

bool World::collides(Vec3 feet) const {
    if (feet.y + playerHeight < 0 || feet.y > height) {
        return false;
    }
    int bottom = int(std::floor(feet.y + collisionEpsilon));
    int top = int(std::floor(feet.y + playerHeight - collisionEpsilon));
    int north = int(std::floor(feet.z - playerRadius + collisionEpsilon));
    int south = int(std::floor(feet.z + playerRadius - collisionEpsilon));
    int west = int(std::floor(feet.x - playerRadius + collisionEpsilon));
    int east = int(std::floor(feet.x + playerRadius - collisionEpsilon));

    for (int y = bottom; y <= top; ++y) {
        for (int z = north; z <= south; ++z) {
            for (int x = west; x <= east; ++x) {
                if (solid(get(x, y, z))) {
                    return true;
                }
            }
        }
    }
    return false;
}

Vec3 World::spawn() const {
    return {0.5f, float(surfaceHeight(0, 0)) + 1.01f, 0.5f};
}

void World::move(Player& player, Input input, float dt) const {
    dt = std::clamp(dt, 0.0f, maximumFrameTime);
    float length = std::hypot(input.forward, input.right);
    if (length > 1) {
        input.forward /= length;
        input.right /= length;
    }

    float speed = player.flying ? (input.sprint ? fastFlyingSpeed : flyingSpeed)
                                : (input.sprint ? sprintingSpeed : walkingSpeed);
    Vec3 velocity{
        (std::sin(player.yaw) * input.forward - std::cos(player.yaw) * input.right) * speed,
        0,
        (std::cos(player.yaw) * input.forward + std::sin(player.yaw) * input.right) * speed};
    if (player.flying) {
        player.verticalVelocity = 0;
        velocity.y = input.vertical * speed;
    } else {
        if (input.jump && player.grounded) {
            player.verticalVelocity = jumpSpeed;
        }
        player.verticalVelocity =
            std::max(player.verticalVelocity - gravity * dt, terminalVelocity);
        velocity.y = player.verticalVelocity;
    }

    int steps = std::max(
        1,
        int(std::ceil(std::max({std::abs(velocity.x), std::abs(velocity.y), std::abs(velocity.z)}) *
                      dt / maximumMovementStep)));
    player.grounded = false;

    for (int step = 0; step < steps; ++step) {
        float timeSlice = dt / float(steps);
        moveAlongAxis(player, {velocity.x * timeSlice, 0, 0});
        moveAlongAxis(player, {0, velocity.y * timeSlice, 0});
        moveAlongAxis(player, {0, 0, velocity.z * timeSlice});
    }
}

void World::moveAlongAxis(Player& player, Vec3 displacement) const {
    auto displaced = [](Vec3 position, Vec3 amount, float fraction = 1.0f) {
        position.x += amount.x * fraction;
        position.y += amount.y * fraction;
        position.z += amount.z * fraction;
        return position;
    };

    Vec3 target = displaced(player.feet, displacement);
    if (!collides(target)) {
        player.feet = target;
        return;
    }

    float clearFraction = 0;
    float blockedFraction = 1;
    for (int i = 0; i < collisionSearchIterations; ++i) {
        float candidateFraction = (clearFraction + blockedFraction) * 0.5f;
        if (collides(displaced(player.feet, displacement, candidateFraction))) {
            blockedFraction = candidateFraction;
        } else {
            clearFraction = candidateFraction;
        }
    }

    player.feet = displaced(player.feet, displacement, clearFraction);
    if (displacement.y != 0) {
        player.grounded = displacement.y < 0;
        player.verticalVelocity = 0;
    }
}

std::optional<Hit> World::raycast(Vec3 origin, Vec3 direction, float reach) const {
    if (origin.y < -reach || origin.y > height + reach) {
        return {};
    }

    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y +
                             direction.z * direction.z);
    if (length < directionEpsilon || !std::isfinite(length)) {
        return {};
    }

    direction.x /= length;
    direction.y /= length;
    direction.z /= length;

    Cell cell{int(std::floor(origin.x)), int(std::floor(origin.y)), int(std::floor(origin.z))};
    Cell previous = cell;
    auto step = [](float value) { return value > 0 ? 1 : value < 0 ? -1 : 0; };
    int stepX = step(direction.x);
    int stepY = step(direction.y);
    int stepZ = step(direction.z);
    auto delta = [](float value) {
        return value == 0 ? std::numeric_limits<float>::infinity() : std::abs(1 / value);
    };
    float deltaX = delta(direction.x);
    float deltaY = delta(direction.y);
    float deltaZ = delta(direction.z);
    auto first = [](float position, int coordinate, float directionValue, int directionStep) {
        return directionStep == 0
                   ? std::numeric_limits<float>::infinity()
                   : (float(coordinate + (directionStep > 0)) - position) / directionValue;
    };
    float nextX = first(origin.x, cell.x, direction.x, stepX);
    float nextY = first(origin.y, cell.y, direction.y, stepY);
    float nextZ = first(origin.z, cell.z, direction.z, stepZ);
    float travelled = 0;
    while (travelled <= reach) {
        Block block = get(cell.x, cell.y, cell.z);
        if (solid(block)) {
            return Hit{cell, previous, block, travelled};
        }
        previous = cell;
        if (nextX <= nextY && nextX <= nextZ) {
            cell.x += stepX;
            travelled = nextX;
            nextX += deltaX;
        } else if (nextY <= nextZ) {
            cell.y += stepY;
            travelled = nextY;
            nextY += deltaY;
        } else {
            cell.z += stepZ;
            travelled = nextZ;
            nextZ += deltaZ;
        }
    }
    return {};
}

} // namespace codesoc
