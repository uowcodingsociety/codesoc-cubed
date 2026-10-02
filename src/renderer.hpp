#pragma once

#include <unordered_map>

#include "raylib.h"
#include "world.hpp"

namespace codesoc {

Color blockColor(Block block, BlockFace face = BlockFace::Top);

/** Builds and draws exposed-face meshes for the world's active chunks. */
class Renderer {
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    /** Rebuilds up to maximumChunks missing or dirty meshes, nearest to the player first.
     *  Clears a chunk's dirty flag after its mesh is rebuilt.
     */
    void rebuild(World& world, Vec3 playerPosition, int maximumChunks = 2);

    void draw(Camera3D camera) const;

    void icon(Block block, float x, float y, float size) const;

    int triangles() const {
        return triangles_;
    }

    static Color sky();

private:
    std::unordered_map<ChunkCoord, Mesh, ChunkCoordHash> meshes_;
    Material material_ = {};
    Shader shader_ = {};
    int eyeLocation_ = 0;
    int fogLocation_ = 0;
    int triangles_ = 0;
};

} // namespace codesoc
