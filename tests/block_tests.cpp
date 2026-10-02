#include <array>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include "blocks.hpp"

using namespace codesoc;

static_assert(sizeof(Block) == 1);
static_assert(Block{} == Block::Air);
static_assert(blockId("Empty") == Block::Air);
static_assert(std::string_view(blockName(blockId("Grass"))) == "Grass");

namespace {

void check(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void testDefinitions() {
    constexpr auto duplicates = std::to_array<BlockDefinition>({
        {"Ruby", {200, 30, 60}},
        {"Ruby", {100, 20, 30}},
    });
    constexpr auto empty = std::to_array<BlockDefinition>({{"", {0, 0, 0}}});
    constexpr auto reserved = std::to_array<BlockDefinition>({{"Empty", {0, 0, 0}}});
    constexpr auto missing = std::to_array<BlockDefinition>({{nullptr, {0, 0, 0}}});
    constexpr std::array<BlockDefinition, 255> tooMany = {};
    static_assert(!validBlockDefinitions(duplicates));
    static_assert(!validBlockDefinitions(empty));
    static_assert(!validBlockDefinitions(reserved));
    static_assert(!validBlockDefinitions(missing));
    static_assert(!validBlockDefinitions(tooMany));

    check(validBlock(Block::Air) && !solid(Block::Air), "Air must remain valid and empty");
    check(!validBlock(Block::Invalid) && !solid(Block::Invalid),
          "Invalid IDs must not become solid blocks");
    check(std::string_view(blockName(Block::Invalid)) == "Unknown",
          "Invalid IDs must have a safe name fallback");
    for (std::size_t i = 0; i < blockDefinitions.size(); ++i) {
        Block block = static_cast<Block>(i + 1);
        check(solid(block), "Every definition must be a solid cube");
        check(std::string_view(blockName(block)) == blockDefinitions[i].name,
              "Each ID must expose its definition's display name");
    }
}

void testFaceColors() {
    constexpr BlockDefinition uniform = {"Ruby", {200, 30, 60}};
    constexpr BlockDefinition layered = {"Layered",
                                         {200, 30, 60},
                                         Rgb{250, 80, 90},
                                         Rgb{100, 10, 20}};
    for (BlockFace face : {BlockFace::Side, BlockFace::Top, BlockFace::Bottom}) {
        check(uniform.colorFor(face) == Rgb{200, 30, 60},
              "An omitted override must use the default colour");
    }
    check(layered.colorFor(BlockFace::Side) == Rgb{200, 30, 60},
          "Overrides must not change side colours");
    check(layered.colorFor(BlockFace::Top) == Rgb{250, 80, 90},
          "Top faces must use the top override");
    check(layered.colorFor(BlockFace::Bottom) == Rgb{100, 10, 20},
          "Bottom faces must use the bottom override");
    check(blockRgb(blockId("Grass"), BlockFace::Top) == Rgb{104, 163, 74},
          "Grass must preserve its green top");
    for (BlockFace face : {BlockFace::Side, BlockFace::Bottom}) {
        check(blockRgb(blockId("Grass"), face) == Rgb{133, 96, 66},
              "Grass must preserve its dirt sides and bottom");
    }
}

} // namespace

int main() {
    try {
        testDefinitions();
        testFaceColors();
        std::cout << "PASS: block definitions, ID validation and face colours\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
