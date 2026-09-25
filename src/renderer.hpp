#pragma once

#include <unordered_map>

#include "raylib.h"
#include "world.hpp"

namespace codesoc {

Color blockColor(Block block);

/** Builds exposed-face meshes for chunks near the player. */
class Renderer {
public:
    Renderer();
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void rebuild(World& world, Vec3 playerPosition, int maximumChunks = 2);
    void draw(Camera3D camera) const;
    void icon(Block block, float x, float y, float size) const;
    int triangles() const { return triangles_; }
    static Color sky();

private:
    std::unordered_map<ChunkCoord, Mesh, ChunkCoordHash> meshes_;
    Material material_{};
    Shader shader_{};
    int eyeLocation_{}, fogLocation_{}, triangles_{};
};

} // namespace codesoc
