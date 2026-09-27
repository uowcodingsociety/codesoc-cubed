#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

#include "application.hpp"
#include "raylib.h"
#include "raymath.h"
#include "renderer.hpp"
#include "world.hpp"

namespace {
using namespace codesoc;

constexpr int initialWindowWidth = 1440;
constexpr int initialWindowHeight = 900;
constexpr int minimumWindowWidth = 960;
constexpr int minimumWindowHeight = 640;
constexpr int targetFramesPerSecond = 120;
constexpr float maximumFrameTime = 0.05f;
constexpr float blockActionInterval = 0.17f;
constexpr float mouseSensitivity = 0.0025f;
constexpr float maximumLookPitch = 1.55f;
constexpr double doubleSpaceInterval = 0.25;
constexpr Color ink(19, 31, 34, 255);
constexpr Color cream(242, 241, 220, 255);
constexpr Color muted(160, 182, 175, 255);
constexpr Color accent(185, 221, 127, 255);
constexpr Color panel(19, 31, 34, 220);

struct Options {
    std::uint32_t seed = 0;
    bool explicitSeed = false;
    bool smokeTest = false;
};

void label(const char* text, int x, int y, int size, Color color = cream) {
    DrawText(text, x, y, size, color);
}

void centeredLabel(const char* text, int x, int y, int size, Color color = cream) {
    label(text, x - MeasureText(text, size) / 2, y, size, color);
}

void printHelp() {
    std::cout << "Codesoc Cubed\n"
                 "  --seed NUMBER  Reproduce a particular world\n"
                 "  --smoke-test   Run a short rendering and interaction check\n";
}

std::uint32_t parseSeed(const std::string& value) {
    std::uint32_t seed = 0;
    auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), seed);
    if (error != std::errc() || end != value.data() + value.size()) {
        throw std::runtime_error("Seed must be a 32-bit unsigned integer");
    }
    return seed;
}

bool parseOptions(int argc, char** argv, Options& options) {
    for (int i = 1; i < argc; ++i) {
        std::string argument = argv[i];
        if (argument == "--help") {
            printHelp();
            return false;
        }
        if (argument == "--smoke-test") {
            options.smokeTest = true;
        } else if (argument == "--seed") {
            if (++i >= argc) {
                throw std::runtime_error("Missing value for --seed");
            }
            options.seed = parseSeed(argv[i]);
            options.explicitSeed = true;
        } else {
            throw std::runtime_error("Unknown option: " + argument);
        }
    }
    return true;
}

Vector3 toRaylib(Vec3 value) {
    return {value.x, value.y, value.z};
}

class Game {
public:
    explicit Game(Options options) : options_(options), world_(options.seed) {
        world_.preloadSpawn();
        player_.feet = world_.spawn();
        renderer_.rebuild(world_, player_.feet, 9);
    }

    int run() {
        if (!options_.smokeTest) {
            DisableCursor();
        }

        while (!quit_) {
            if (WindowShouldClose()) {
                break;
            }

            ++frameCount_;
            float dt = std::min(GetFrameTime(), maximumFrameTime);
            actionTimer_ -= dt;

            if (options_.smokeTest) {
                smokeStep();
            } else {
                updateGame(dt);
            }

            world_.streamAround(player_.feet);
            renderer_.rebuild(world_, player_.feet);
            drawFrame();

            if (options_.smokeTest && frameCount_ >= 60) {
                quit_ = true;
            }
        }

        EnableCursor();
        if (options_.smokeTest) {
            std::cout << "SMOKE " << (result_ == 0 ? "PASS" : "FAIL") << ": " << frameCount_
                      << " frames, " << renderer_.triangles() << " triangles\n";
        }

        return result_;
    }

private:
    void verify(bool condition, const char* detail) {
        if (condition) {
            return;
        }
        std::cerr << "SMOKE FAIL: " << detail << '\n';
        result_ = 2;
    }

    void pressSpace(double now) {
        if (lastSpacePress_ >= 0 && now - lastSpacePress_ <= doubleSpaceInterval) {
            player_.flying = !player_.flying;
            player_.verticalVelocity = 0;
            lastSpacePress_ = -1;
        } else {
            jumpRequested_ = !player_.flying;
            lastSpacePress_ = now;
        }
    }

    bool placeAt(Cell cell) {
        Block block = palette[std::size_t(player_.selected)];
        if (block == Block::Air || world_.get(cell.x, cell.y, cell.z) != Block::Air ||
            world_.overlapsPlayer(cell, player_.feet)) {
            return false;
        }
        return world_.set(cell.x, cell.y, cell.z, block);
    }

    void smokeStep() {
        if (frameCount_ == 3) {
            verify(world_.loadedCount() >= 9, "spawn area must be loaded before play");
            verify(!world_.collides(player_.feet), "player must start in the world safely");
        } else if (frameCount_ == 12) {
            int surface = world_.surfaceHeight(3, 3);
            Cell cell(3, surface + 1, 3);
            verify(world_.set(cell.x, cell.y, cell.z, Block::Stone),
                   "placement must change terrain");
            verify(world_.set(cell.x, cell.y, cell.z, Block::Air), "mining must remove a block");
            player_.selected = 3;
            verify(!placeAt(cell), "empty hotbar slot must not place a block");
            player_.selected = 0;
        } else if (frameCount_ == 20) {
            pressSpace(1.0);
            pressSpace(1.1);
            verify(player_.flying, "double space must enable flight");
            float start = player_.feet.z;
            world_.move(player_, {1, 0, 0, false, false}, 0.05f);
            verify(player_.feet.z > start, "movement must advance the player");
        } else if (frameCount_ == 30) {
            pressSpace(2.0);
            pressSpace(2.1);
            verify(!player_.flying, "double space must disable flight");
        }
    }

    void updateGame(float dt) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Q)) {
            quit_ = true;
            return;
        }
        if (!IsWindowFocused()) {
            if (cursorCaptured_) {
                EnableCursor();
                cursorCaptured_ = false;
            }
            return;
        }
        if (!cursorCaptured_) {
            DisableCursor();
            cursorCaptured_ = true;
        }

        if (IsKeyPressed(KEY_F11)) {
            ToggleBorderlessWindowed();
        }
        if (IsKeyPressed(KEY_F3)) {
            debugVisible_ = !debugVisible_;
        }
        if (IsKeyPressed(KEY_H)) {
            helpVisible_ = !helpVisible_;
        }

        Vector2 mouse = GetMouseDelta();
        player_.yaw -= mouse.x * mouseSensitivity;
        player_.pitch = std::clamp(player_.pitch - mouse.y * mouseSensitivity,
                                   -maximumLookPitch,
                                   maximumLookPitch);

        if (IsKeyPressed(KEY_SPACE)) {
            pressSpace(GetTime());
        }

        for (int slot = 0; slot < int(palette.size()); ++slot) {
            if (IsKeyPressed(KEY_ONE + slot)) {
                player_.selected = slot;
            }
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            int offset = wheel > 0 ? -1 : 1;
            player_.selected =
                (player_.selected + offset + int(palette.size())) % int(palette.size());
        }

        Input input(float(IsKeyDown(KEY_W) - IsKeyDown(KEY_S)),
                    float(IsKeyDown(KEY_D) - IsKeyDown(KEY_A)),
                    player_.flying ? float(IsKeyDown(KEY_SPACE) - IsKeyDown(KEY_LEFT_CONTROL))
                                   : 0.0f,
                    jumpRequested_,
                    IsKeyDown(KEY_LEFT_SHIFT));
        world_.move(player_, input, dt);

        jumpRequested_ = false;
        updateBlocks();
    }

    void updateBlocks() {
        auto target = world_.raycast(player_.eye(), player_.direction());
        if (!target) {
            return;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
            auto picked = std::find(palette.begin(), palette.end(), target->block);
            if (picked != palette.end()) {
                player_.selected = int(std::distance(palette.begin(), picked));
            }
        }

        if (actionTimer_ > 0) {
            return;
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            world_.set(target->cell.x, target->cell.y, target->cell.z, Block::Air);
            actionTimer_ = blockActionInterval;
        } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            placeAt(target->previous);
            actionTimer_ = blockActionInterval;
        }
    }

    Camera3D camera() const {
        Camera3D result = {};
        result.position = toRaylib(player_.eye());
        result.target = Vector3Add(result.position, toRaylib(player_.direction()));
        result.up = {0, 1, 0};
        result.fovy = 70.0f;
        result.projection = CAMERA_PERSPECTIVE;
        return result;
    }

    void drawFrame() {
        int width = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        Camera3D activeCamera = camera();

        BeginDrawing();
        ClearBackground(Renderer::sky());
        renderer_.draw(activeCamera);

        auto target = world_.raycast(player_.eye(), player_.direction());
        if (target) {
            BeginMode3D(activeCamera);
            Vector3 centre(target->cell.x + 0.5f, target->cell.y + 0.5f, target->cell.z + 0.5f);
            DrawCubeWires(centre, 1.006f, 1.006f, 1.006f, ink);
            EndMode3D();
        }

        drawHud(width, screenHeight, target);
        EndDrawing();
    }

    void drawHud(int width, int screenHeight, const std::optional<Hit>& target) const {
        int centreX = width / 2;
        int centreY = screenHeight / 2;
        DrawRectangle(centreX - 8, centreY, 17, 2, ink);
        DrawRectangle(centreX, centreY - 8, 2, 17, ink);
        DrawLine(centreX - 7, centreY, centreX + 7, centreY, cream);
        DrawLine(centreX, centreY - 7, centreX, centreY + 7, cream);

        DrawRectangle(24, 24, 218, 62, panel);
        label("CODESOC CUBED", 40, 34, 20);
        label(player_.flying ? "CREATIVE / FLYING" : "CREATIVE / WALKING", 40, 61, 10, accent);
        if (target) {
            const char* name = blockName(target->block);
            int labelWidth = MeasureText(name, 16) + 28;
            DrawRectangle(centreX - labelWidth / 2, centreY + 25, labelWidth, 30, panel);
            centeredLabel(name, centreX, centreY + 32, 16);
        }

        constexpr int slotSize = 58;
        int barWidth = int(palette.size()) * slotSize + 12;
        int barX = centreX - barWidth / 2;
        int barY = screenHeight - 86;
        DrawRectangle(barX - 1, barY - 1, barWidth + 2, 72, panel);
        for (int slot = 0; slot < int(palette.size()); ++slot) {
            int x = barX + 6 + slot * slotSize;
            bool selected = slot == player_.selected;
            DrawRectangle(x,
                          barY + 6,
                          slotSize - 4,
                          58,
                          selected ? Color(73, 94, 65, 255) : Color(39, 54, 49, 230));
            if (selected) {
                DrawRectangleLinesEx({float(x), float(barY + 6), float(slotSize - 4), 58},
                                     2,
                                     accent);
            }
            label(TextFormat("%d", slot + 1), x + 5, barY + 11, 10, selected ? accent : muted);
            renderer_.icon(palette[std::size_t(slot)], float(x + 27), float(barY + 21), 29);
        }
        centeredLabel(blockName(palette[std::size_t(player_.selected)]), centreX, barY - 28, 20);
        if (palette[std::size_t(player_.selected)] != Block::Air) {
            renderer_.icon(palette[std::size_t(player_.selected)],
                           float(width - 110),
                           float(screenHeight - 185),
                           110);
        }

        if (helpVisible_) {
            DrawRectangle(24, screenHeight - 140, 280, 116, panel);
            label("WASD MOVE / SPACE JUMP", 38, screenHeight - 128, 12, accent);
            label("DOUBLE SPACE FLY / CTRL DOWN", 38, screenHeight - 103, 12);
            label("SHIFT FAST / MOUSE MINE + PLACE", 38, screenHeight - 78, 12);
            label("ESC OR Q QUIT / H HIDE HELP", 38, screenHeight - 53, 12, muted);
        }
        if (debugVisible_) {
            DrawRectangle(24, 100, 345, 112, panel);
            label(TextFormat("%d FPS / %d triangles", GetFPS(), renderer_.triangles()),
                  38,
                  112,
                  16,
                  accent);
            label(TextFormat("XYZ %.1f %.1f %.1f", player_.feet.x, player_.feet.y, player_.feet.z),
                  38,
                  139,
                  16);
            ChunkCoord chunk =
                World::chunkFor(int(std::floor(player_.feet.x)), int(std::floor(player_.feet.z)));
            label(TextFormat("Seed %u / chunk %d,%d / loaded %zu",
                             world_.seed(),
                             chunk.x,
                             chunk.z,
                             world_.loadedCount()),
                  38,
                  166,
                  14);
        }
    }

    Options options_;
    World world_;
    Player player_;
    Renderer renderer_;
    double lastSpacePress_ = -1;
    float actionTimer_ = 0;
    int frameCount_ = 0;
    int result_ = 0;
    bool jumpRequested_ = false;
    bool cursorCaptured_ = true;
    bool debugVisible_ = false;
    bool helpVisible_ = true;
    bool quit_ = false;
};

bool openWindow(bool smokeTest) {
    SetTraceLogLevel(LOG_WARNING);
    unsigned int flags = FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT;
    if (smokeTest) {
        flags |= FLAG_WINDOW_HIDDEN;
    }
    SetConfigFlags(flags);
    InitWindow(initialWindowWidth, initialWindowHeight, "Codesoc Cubed");
    if (!IsWindowReady()) {
        return false;
    }

    SetWindowMinSize(minimumWindowWidth, minimumWindowHeight);
    SetExitKey(KEY_NULL);
    SetTargetFPS(targetFramesPerSecond);
    return true;
}

} // namespace

int codesoc::runApplication(int argc, char** argv) {
    Options options;
    
    try {
        if (!parseOptions(argc, argv, options)) {
            return 0;
        }
        
        if (!options.explicitSeed) {
            options.seed = std::random_device{}();
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    if (!openWindow(options.smokeTest)) {
        return 1;
    }

    int result;
    {
        Game game(options);
        result = game.run();
    }

    CloseWindow();
    
    return result;
}
