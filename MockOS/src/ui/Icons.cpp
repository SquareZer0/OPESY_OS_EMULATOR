#include "ui/Icons.h"

#include <cmath>

namespace icons {

namespace {

constexpr float PI = 3.14159265358979f;
constexpr ImU32 WHITE = IM_COL32(255, 255, 255, 255);

// Angle pointing straight up. ImGui's y axis points down, so up is -90 degrees.
constexpr float UP = -PI * 0.5f;

ImVec2 at(const ImVec2& min, float size, float x, float y) {
    return ImVec2(min.x + size * x, min.y + size * y);
}

// The rounded tile behind every app icon.
void tile(ImDrawList* drawList, const ImVec2& min, float size, ImU32 color) {
    const ImVec2 max(min.x + size, min.y + size);
    const float rounding = size * 0.26f;
    drawList->AddRectFilled(min, max, color, rounding);
    drawList->AddRect(min, max, IM_COL32(255, 255, 255, 140), rounding, 0, 1.0f);
}

}  // namespace

void wifi(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color) {
    // Three arcs fanning up from a dot at the bottom.
    const ImVec2 origin(center.x, center.y + size * 0.30f);
    const float thickness = size * 0.09f;
    const float radii[] = {0.20f, 0.38f, 0.56f};
    for (float radius : radii) {
        drawList->PathArcTo(origin, size * radius, UP - PI * 0.25f, UP + PI * 0.25f);
        drawList->PathStroke(color, 0, thickness);
    }
    drawList->AddCircleFilled(origin, size * 0.07f, color);
}

void speaker(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color, bool muted) {
    const float s = size;
    const float left = center.x - s * 0.38f;
    // Box, then the cone widening to the right.
    drawList->AddRectFilled(ImVec2(left, center.y - s * 0.12f), ImVec2(left + s * 0.16f, center.y + s * 0.12f), color, 1.0f);
    const ImVec2 cone[] = {
        ImVec2(left + s * 0.16f, center.y - s * 0.12f),
        ImVec2(left + s * 0.38f, center.y - s * 0.32f),
        ImVec2(left + s * 0.38f, center.y + s * 0.32f),
        ImVec2(left + s * 0.16f, center.y + s * 0.12f),
    };
    drawList->AddConvexPolyFilled(cone, 4, color);

    const float thickness = s * 0.08f;
    if (muted) {
        const float x0 = center.x + s * 0.12f;
        const float x1 = center.x + s * 0.38f;
        drawList->AddLine(ImVec2(x0, center.y - s * 0.13f), ImVec2(x1, center.y + s * 0.13f), color, thickness);
        drawList->AddLine(ImVec2(x0, center.y + s * 0.13f), ImVec2(x1, center.y - s * 0.13f), color, thickness);
    } else {
        const ImVec2 origin(center.x - s * 0.02f, center.y);
        drawList->PathArcTo(origin, s * 0.22f, -PI * 0.28f, PI * 0.28f);
        drawList->PathStroke(color, 0, thickness);
        drawList->PathArcTo(origin, s * 0.40f, -PI * 0.28f, PI * 0.28f);
        drawList->PathStroke(color, 0, thickness);
    }
}

void power(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color) {
    // A ring with a gap at the top, and a line through the gap.
    const float thickness = size * 0.10f;
    const float gap = 0.65f;
    drawList->PathArcTo(center, size * 0.36f, UP + gap, UP + 2.0f * PI - gap);
    drawList->PathStroke(color, 0, thickness);
    drawList->AddLine(ImVec2(center.x, center.y - size * 0.46f), ImVec2(center.x, center.y - size * 0.06f), color,
                      thickness);
}

void processes(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color) {
    // A 2 x 2 grid of app squares.
    const float thickness = size * 0.10f;
    const float side = size * 0.34f;
    const float offsets[] = {-0.40f, 0.06f};
    for (float dy : offsets) {
        for (float dx : offsets) {
            const ImVec2 min(center.x + size * dx, center.y + size * dy);
            drawList->AddRect(min, ImVec2(min.x + side, min.y + side), color, size * 0.08f, 0, thickness);
        }
    }
}

void performance(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color) {
    // A speedometer: the top half of a ring and a needle.
    const float thickness = size * 0.10f;
    const ImVec2 hub(center.x, center.y + size * 0.16f);
    drawList->PathArcTo(hub, size * 0.42f, PI, 2.0f * PI);
    drawList->PathStroke(color, 0, thickness);
    drawList->AddLine(hub, ImVec2(hub.x + size * 0.20f, hub.y - size * 0.20f), color, thickness);
    drawList->AddCircleFilled(hub, size * 0.08f, color);
}

void appHistory(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color) {
    // A clock.
    const float thickness = size * 0.10f;
    drawList->AddCircle(center, size * 0.42f, color, 0, thickness);
    drawList->AddLine(center, ImVec2(center.x, center.y - size * 0.24f), color, thickness);
    drawList->AddLine(center, ImVec2(center.x + size * 0.18f, center.y), color, thickness);
}

void startupApps(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color) {
    // A rocket: pointed body, two fins, a window and a flame.
    const float thickness = size * 0.09f;
    auto p = [&](float x, float y) { return ImVec2(center.x + size * x, center.y + size * y); };
    const ImVec2 body[] = {p(0.0f, -0.48f), p(0.15f, -0.20f), p(0.15f, 0.20f), p(-0.15f, 0.20f), p(-0.15f, -0.20f)};
    drawList->AddPolyline(body, 5, color, ImDrawFlags_Closed, thickness);
    const ImVec2 leftFin[] = {p(-0.15f, 0.00f), p(-0.32f, 0.24f), p(-0.15f, 0.20f)};
    const ImVec2 rightFin[] = {p(0.15f, 0.00f), p(0.32f, 0.24f), p(0.15f, 0.20f)};
    drawList->AddPolyline(leftFin, 3, color, 0, thickness);
    drawList->AddPolyline(rightFin, 3, color, 0, thickness);
    drawList->AddCircleFilled(p(0.0f, -0.10f), size * 0.06f, color);
    drawList->AddLine(p(0.0f, 0.30f), p(0.0f, 0.46f), color, thickness);
}

void details(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color) {
    // A bulleted list.
    const float thickness = size * 0.10f;
    const float rows[] = {-0.28f, 0.0f, 0.28f};
    for (float dy : rows) {
        const float y = center.y + size * dy;
        drawList->AddCircleFilled(ImVec2(center.x - size * 0.34f, y), size * 0.07f, color);
        drawList->AddLine(ImVec2(center.x - size * 0.16f, y), ImVec2(center.x + size * 0.42f, y), color, thickness);
    }
}

void services(ImDrawList* drawList, const ImVec2& center, float size, ImU32 color) {
    // A gear: a ring with eight teeth and a hub.
    const float thickness = size * 0.10f;
    drawList->AddCircle(center, size * 0.25f, color, 0, thickness);
    for (int i = 0; i < 8; ++i) {
        const float angle = i * PI * 0.25f;
        const ImVec2 dir(std::cos(angle), std::sin(angle));
        drawList->AddLine(ImVec2(center.x + dir.x * size * 0.29f, center.y + dir.y * size * 0.29f),
                          ImVec2(center.x + dir.x * size * 0.44f, center.y + dir.y * size * 0.44f), color,
                          size * 0.15f);
    }
    drawList->AddCircleFilled(center, size * 0.08f, color);
}

void calculator(ImDrawList* drawList, const ImVec2& min, float size) {
    tile(drawList, min, size, IM_COL32(118, 196, 184, 255));
    // Display along the top, then a 4 x 3 grid of keys.
    drawList->AddRectFilled(at(min, size, 0.24f, 0.20f), at(min, size, 0.76f, 0.36f), WHITE, size * 0.03f);
    const float key = 0.10f;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 4; ++col) {
            const float x = 0.24f + col * (key + 0.04f);
            const float y = 0.44f + row * (key + 0.03f);
            drawList->AddRectFilled(at(min, size, x, y), at(min, size, x + key, y + key), WHITE, size * 0.025f);
        }
    }
}

void notepad(ImDrawList* drawList, const ImVec2& min, float size) {
    const ImU32 color = IM_COL32(244, 190, 112, 255);
    tile(drawList, min, size, color);
    // A white page with lines of text.
    drawList->AddRectFilled(at(min, size, 0.28f, 0.20f), at(min, size, 0.72f, 0.80f), WHITE, size * 0.05f);
    const float thickness = size * 0.045f;
    const float lines[] = {0.34f, 0.45f, 0.56f};
    for (float y : lines) {
        drawList->AddLine(at(min, size, 0.36f, y), at(min, size, 0.64f, y), color, thickness);
    }
    drawList->AddLine(at(min, size, 0.36f, 0.67f), at(min, size, 0.54f, 0.67f), color, thickness);
}

void taskManager(ImDrawList* drawList, const ImVec2& min, float size) {
    tile(drawList, min, size, IM_COL32(158, 140, 228, 255));
    // A bar chart.
    const float heights[] = {0.22f, 0.40f, 0.30f, 0.50f};
    const float width = 0.09f;
    const float baseline = 0.76f;
    for (int i = 0; i < 4; ++i) {
        const float x = 0.235f + i * (width + 0.055f);
        drawList->AddRectFilled(at(min, size, x, baseline - heights[i]), at(min, size, x + width, baseline), WHITE,
                                size * 0.02f);
    }
}

}  // namespace icons
