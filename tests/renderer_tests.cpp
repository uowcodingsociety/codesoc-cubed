#include <array>
#include <iostream>

// Exercise the same face builder used by chunk meshes, without opening a window.
#include "../src/renderer.cpp"

int main() {
    constexpr std::array<int, 6> faceAxis = {0, 0, 1, 1, 2, 2};
    constexpr std::array<float, 6> facePlane = {1, 0, 1, 0, 1, 0};

    for (int face = 0; face < 6; ++face) {
        codesoc::Builder builder;
        codesoc::appendFace(builder, codesoc::Block::Stone, 0, 0, 0, face);

        for (int vertex = 0; vertex < 6; ++vertex) {
            float coordinate = builder.vertices[std::size_t(vertex * 3 + faceAxis[face])];

            if (coordinate != facePlane[face]) {
                std::cerr << "Face " << face << " has a vertex off its block boundary\n";
                return 1;
            }
        }

        auto point = [&](int vertex, int axis) {
            return builder.vertices[std::size_t(vertex * 3 + axis)];
        };

        int u = (faceAxis[face] + 1) % 3;
        int v = (faceAxis[face] + 2) % 3;
        float cross = (point(1, u) - point(0, u)) * (point(2, v) - point(0, v)) -
                      (point(1, v) - point(0, v)) * (point(2, u) - point(0, u));
        float sign = face % 2 == 0 ? 1.0f : -1.0f;

        if (cross * sign <= 0) {
            std::cerr << "Face " << face << " has reversed or degenerate winding\n";
            return 1;
        }
    }
}
