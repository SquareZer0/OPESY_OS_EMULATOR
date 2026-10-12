#include "ui/Wallpaper.h"

#include <cmath>

namespace wallpaper {

namespace {

// A soft patch of color. x and y are fractions of the screen's width and
// height; radius is a fraction of its height.
struct Blob {
    float x;
    float y;
    float radius;
    float r, g, b;
};

// Colors blend toward these blobs, like a mesh gradient.
const Blob BLOBS[] = {
    {0.00f, 0.05f, 0.65f, 166, 233, 220},  // mint, top left
    {0.30f, 0.20f, 0.45f, 188, 206, 238},  // powder blue
    {0.50f, 0.00f, 0.50f, 218, 196, 240},  // lavender, top
    {0.80f, 0.15f, 0.50f, 236, 198, 222},  // pink
    {1.00f, 0.00f, 0.40f, 214, 186, 232},  // lilac, top right
    {0.68f, 0.82f, 0.45f, 252, 234, 156},  // butter yellow
    {1.00f, 0.90f, 0.45f, 250, 214, 172},  // peach, bottom right
    {0.02f, 0.95f, 0.55f, 164, 222, 214},  // seafoam, bottom left
};

// Color where no blob reaches, and how strongly it holds its ground.
constexpr float BASE_R = 242.0f;
constexpr float BASE_G = 235.0f;
constexpr float BASE_B = 247.0f;
constexpr float BASE_WEIGHT = 0.15f;

// The wallpaper is drawn as a grid of cells; the GPU blends colors smoothly
// across each cell, so a coarse grid is enough for soft gradients.
constexpr int GRID_COLUMNS = 48;
constexpr int GRID_ROWS = 28;

ImU32 colorAt(float u, float v, float aspect) {
    float r = BASE_R * BASE_WEIGHT;
    float g = BASE_G * BASE_WEIGHT;
    float b = BASE_B * BASE_WEIGHT;
    float total = BASE_WEIGHT;
    for (const Blob& blob : BLOBS) {
        // Measure in screen heights so blobs stay round on wide windows.
        const float dx = (u - blob.x) * aspect;
        const float dy = v - blob.y;
        const float weight = std::exp(-2.5f * (dx * dx + dy * dy) / (blob.radius * blob.radius));
        r += blob.r * weight;
        g += blob.g * weight;
        b += blob.b * weight;
        total += weight;
    }
    return IM_COL32(static_cast<int>(r / total), static_cast<int>(g / total), static_cast<int>(b / total), 255);
}

}  // namespace

void draw(ImDrawList* drawList, const ImVec2& min, const ImVec2& max) {
    const float width = max.x - min.x;
    const float height = max.y - min.y;
    if (width <= 0.0f || height <= 0.0f) {
        return;
    }
    const float aspect = width / height;
    const ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
    const int columns = GRID_COLUMNS + 1;  // vertices per row

    drawList->PrimReserve(GRID_COLUMNS * GRID_ROWS * 6, columns * (GRID_ROWS + 1));
    // Read after PrimReserve(), which may start a new batch and reset it.
    const unsigned int first = drawList->_VtxCurrentIdx;

    for (int row = 0; row <= GRID_ROWS; ++row) {
        const float v = static_cast<float>(row) / GRID_ROWS;
        for (int col = 0; col <= GRID_COLUMNS; ++col) {
            const float u = static_cast<float>(col) / GRID_COLUMNS;
            drawList->PrimWriteVtx(ImVec2(min.x + u * width, min.y + v * height), uv, colorAt(u, v, aspect));
        }
    }
    for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLUMNS; ++col) {
            const unsigned int topLeft = first + row * columns + col;
            const unsigned int bottomLeft = topLeft + columns;
            drawList->PrimWriteIdx(static_cast<ImDrawIdx>(topLeft));
            drawList->PrimWriteIdx(static_cast<ImDrawIdx>(topLeft + 1));
            drawList->PrimWriteIdx(static_cast<ImDrawIdx>(bottomLeft + 1));
            drawList->PrimWriteIdx(static_cast<ImDrawIdx>(topLeft));
            drawList->PrimWriteIdx(static_cast<ImDrawIdx>(bottomLeft + 1));
            drawList->PrimWriteIdx(static_cast<ImDrawIdx>(bottomLeft));
        }
    }
}

}  // namespace wallpaper
