#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace codesoc {

/** Air is zero so newly initialized chunk storage is empty. */
enum class Block : std::uint8_t { Air = 0, Invalid = 255 };
enum class BlockFace { Side, Top, Bottom };

struct Rgb {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    bool operator==(const Rgb&) const = default;
};

struct BlockDefinition {
    const char* name;
    Rgb color;
    std::optional<Rgb> top = {};
    std::optional<Rgb> bottom = {};

    constexpr Rgb colorFor(BlockFace face) const {
        if (face == BlockFace::Top) {
            return top.value_or(color);
        }

        if (face == BlockFace::Bottom) {
            return bottom.value_or(color);
        }

        return color;
    }
};

/** Add solid cubes here. Omitted face overrides use the default colour. */
inline constexpr auto blockDefinitions = std::to_array<BlockDefinition>({
    {"Grass", {133, 96, 66}, Rgb{104, 163, 74}},
    {"Dirt", {133, 96, 66}},
    {"Stone", {137, 145, 151}},
});

constexpr bool validBlockDefinitions(std::span<const BlockDefinition> definitions) {
    // Reserve zero for Air and 255 for Invalid.
    if (definitions.size() >= std::size_t(Block::Invalid)) {
        return false;
    }

    for (std::size_t i = 0; i < definitions.size(); ++i) {
        if (definitions[i].name == nullptr) {
            return false;
        }

        std::string_view name = definitions[i].name;

        if (name.empty() || name == "Empty") {
            return false;
        }

        for (std::size_t j = 0; j < i; ++j) {
            if (name == definitions[j].name) {
                return false;
            }
        }
    }

    return true;
}

static_assert(
    validBlockDefinitions(blockDefinitions),
    "Block definitions need unique nonempty names other than Empty, and at most 254 entries");

/** Resolve a literal name during compilation, rejecting misspelled or missing names. */
consteval Block blockId(std::string_view name) {
    if (name == "Empty") {
        return Block::Air;
    }

    for (std::size_t i = 0; i < blockDefinitions.size(); ++i) {
        if (name == blockDefinitions[i].name) {
            return static_cast<Block>(i + 1);
        }
    }

    throw "Unknown block name";
}

constexpr bool validBlock(Block block) {
    return std::size_t(block) <= blockDefinitions.size();
}

constexpr bool solid(Block block) {
    return block != Block::Air && validBlock(block);
}

constexpr const char* blockName(Block block) {
    if (block == Block::Air) {
        return "Empty";
    }

    return validBlock(block) ? blockDefinitions[std::size_t(block) - 1].name : "Unknown";
}

constexpr Rgb blockRgb(Block block, BlockFace face) {
    return solid(block) ? blockDefinitions[std::size_t(block) - 1].colorFor(face) : Rgb{};
}

} // namespace codesoc
