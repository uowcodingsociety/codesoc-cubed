#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <raylib.h>
#include <raymath.h>
#include <vector>

#include "renderer.hpp"

namespace codesoc {
namespace {

constexpr std::array<Cell, 6> normals{
    {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}}};
constexpr float corners[6][4][3] = {{{1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1}},
                                    {{0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0}},
                                    {{0, 1, 1}, {1, 1, 1}, {1, 1, 0}, {0, 1, 0}},
                                    {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}},
                                    {{0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}},
                                    {{1, 0, 0}, {0, 0, 0}, {0, 1, 0}, {1, 1, 0}}};
constexpr std::array<float, 6> faceLight{0.78f, 0.68f, 1.0f, 0.55f, 0.88f, 0.75f};

constexpr const char* vertexShader = R"(
#version 330
in vec3 vertexPosition;
in vec4 vertexColor;
uniform mat4 mvp;
out vec4 fragColor;
out vec3 worldPosition;
void main() {
    fragColor = vertexColor;
    worldPosition = vertexPosition;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
})";

constexpr const char* fragmentShader = R"(
#version 330
in vec4 fragColor;
in vec3 worldPosition;
uniform vec3 eye;
uniform vec3 fogColor;
out vec4 finalColor;
void main() {
    float fog = smoothstep(48.0, 72.0, length(worldPosition - eye));
    finalColor = vec4(mix(fragColor.rgb, fogColor, fog), 1.0);
})";

Color shade(Color base, float light) {
    return {std::uint8_t(std::clamp(base.r * light, 0.0f, 255.0f)),
            std::uint8_t(std::clamp(base.g * light, 0.0f, 255.0f)),
            std::uint8_t(std::clamp(base.b * light, 0.0f, 255.0f)),
            base.a};
}

struct Builder {
    std::vector<float> vertices;
    std::vector<float> uv;
    std::vector<unsigned char> colors;

    void vertex(Vector3 position, Color color) {
        vertices.insert(vertices.end(), {position.x, position.y, position.z});
        uv.insert(uv.end(), {0.0f, 0.0f});
        colors.insert(colors.end(), {color.r, color.g, color.b, color.a});
    }

    Mesh upload() const {
        Mesh mesh{};
        mesh.vertexCount = int(vertices.size() / 3);
        mesh.triangleCount = mesh.vertexCount / 3;
        auto copy = [](const auto& data) {
            auto bytes = data.size() * sizeof(data[0]);
            void* memory = MemAlloc(unsigned(bytes));
            std::memcpy(memory, data.data(), bytes);
            return memory;
        };
        mesh.vertices = static_cast<float*>(copy(vertices));
        mesh.texcoords = static_cast<float*>(copy(uv));
        mesh.colors = static_cast<unsigned char*>(copy(colors));
        UploadMesh(&mesh, false);
        return mesh;
    }
};

void appendFace(Builder& builder, Block block, int x, int y, int z, int face) {
    constexpr std::array indices{0, 1, 2, 0, 2, 3};
    Color base = block == Block::Grass && face != 2 ? blockColor(Block::Dirt) : blockColor(block);
    Color color = shade(base, faceLight[std::size_t(face)]);
    for (int index : indices) {
        const float* corner = corners[face][index];
        builder.vertex({x + corner[0], y + corner[1], z + corner[2]}, color);
    }
}

Mesh buildChunk(const World& world, ChunkCoord coord) {
    Builder builder;
    for (int y = 0; y < height; ++y) {
        for (int localZ = 0; localZ < chunkSize; ++localZ) {
            for (int localX = 0; localX < chunkSize; ++localX) {
                int x = coord.x * chunkSize + localX;
                int z = coord.z * chunkSize + localZ;
                Block block = world.get(x, y, z);
                if (block == Block::Air) {
                    continue;
                }
                for (int face = 0; face < int(normals.size()); ++face) {
                    Cell normal = normals[std::size_t(face)];
                    if (!solid(world.get(x + normal.x, y + normal.y, z + normal.z))) {
                        appendFace(builder, block, x, y, z, face);
                    }
                }
            }
        }
    }
    return builder.vertices.empty() ? Mesh{} : builder.upload();
}

} // namespace

Color blockColor(Block block) {
    switch (block) {
    case Block::Grass:
        return {104, 163, 74, 255};
    case Block::Dirt:
        return {133, 96, 66, 255};
    case Block::Stone:
        return {137, 145, 151, 255};
    default:
        return BLANK;
    }
}

Renderer::Renderer() {
    shader_ = LoadShaderFromMemory(vertexShader, fragmentShader);
    material_ = LoadMaterialDefault();
    material_.shader = shader_;
    eyeLocation_ = GetShaderLocation(shader_, "eye");
    fogLocation_ = GetShaderLocation(shader_, "fogColor");
}

Renderer::~Renderer() {
    for (auto& [coord, mesh] : meshes_) {
        static_cast<void>(coord);
        if (mesh.vertexCount > 0) {
            UnloadMesh(mesh);
        }
    }
    UnloadMaterial(material_);
}

void Renderer::rebuild(World& world, Vec3 playerPosition, int maximumChunks) {
    for (auto it = meshes_.begin(); it != meshes_.end();) {
        if (world.activeChunks().contains(it->first)) {
            ++it;
            continue;
        }
        if (it->second.vertexCount > 0) {
            UnloadMesh(it->second);
        }
        it = meshes_.erase(it);
    }

    std::vector<ChunkCoord> pending;
    for (ChunkCoord coord : world.activeChunks()) {
        if (!meshes_.contains(coord) || world.isDirty(coord)) {
            pending.push_back(coord);
        }
    }
    std::sort(pending.begin(), pending.end(), [&](ChunkCoord a, ChunkCoord b) {
        auto squaredDistance = [&](ChunkCoord coord) {
            float dx = float(coord.x * chunkSize + chunkSize / 2) - playerPosition.x;
            float dz = float(coord.z * chunkSize + chunkSize / 2) - playerPosition.z;
            return dx * dx + dz * dz;
        };
        return squaredDistance(a) < squaredDistance(b);
    });

    for (int i = 0; i < std::min(maximumChunks, int(pending.size())); ++i) {
        ChunkCoord coord = pending[std::size_t(i)];
        auto old = meshes_.find(coord);
        if (old != meshes_.end() && old->second.vertexCount > 0) {
            UnloadMesh(old->second);
        }
        meshes_[coord] = buildChunk(world, coord);
        world.clearDirty(coord);
    }

    triangles_ = 0;
    for (const auto& [coord, mesh] : meshes_) {
        static_cast<void>(coord);
        triangles_ += mesh.triangleCount;
    }
}

Color Renderer::sky() {
    return {153, 196, 219, 255};
}

void Renderer::draw(Camera3D camera) const {
    Color color = sky();
    float fog[3]{color.r / 255.0f, color.g / 255.0f, color.b / 255.0f};
    SetShaderValue(shader_, eyeLocation_, &camera.position, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader_, fogLocation_, fog, SHADER_UNIFORM_VEC3);
    BeginMode3D(camera);
    Matrix identity = MatrixIdentity();
    for (const auto& [coord, mesh] : meshes_) {
        static_cast<void>(coord);
        if (mesh.vertexCount > 0) {
            DrawMesh(mesh, material_, identity);
        }
    }
    EndMode3D();
}

void Renderer::icon(Block block, float x, float y, float size) const {
    if (block == Block::Air) {
        return;
    }
    Color base = blockColor(block);
    Vector2 top{x, y};
    Vector2 left{x - size * 0.5f, y + size * 0.25f};
    Vector2 mid{x, y + size * 0.5f};
    Vector2 right{x + size * 0.5f, y + size * 0.25f};
    Vector2 bottom{x, y + size};
    Vector2 bl{x - size * 0.5f, y + size * 0.75f};
    Vector2 br{x + size * 0.5f, y + size * 0.75f};
    DrawTriangle(top, left, mid, shade(base, 1.15f));
    DrawTriangle(top, mid, right, shade(base, 1.15f));
    DrawTriangle(left, bl, bottom, shade(base, 0.72f));
    DrawTriangle(left, bottom, mid, shade(base, 0.72f));
    DrawTriangle(mid, bottom, br, base);
    DrawTriangle(mid, br, right, base);
}

} // namespace codesoc
