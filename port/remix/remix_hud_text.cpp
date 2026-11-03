/**
 * remix_hud_text.cpp - see remix_hud_text.h.
 *
 * The overlay is a GuiWindow without an ImGui window of its own, like remix_input_display: it draws on the foreground
 * draw list. A label stays visible for the ImGui frame it was put in and the next one.
 */

#include "remix_hud_text.h"

#ifdef REGION_REMIX

#include <imgui.h>
#include <libultraship/libultraship.h>

#include "remix_font.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace {

constexpr float kN64ScreenWidth = 320.0f;
constexpr float kN64ScreenHeight = 240.0f;
constexpr int kLabelLifeFrames = 2;
constexpr uint32_t kShadowColor = IM_COL32(0, 0, 0, 200);

struct HudLabel {
    bool active = false;
    int lastFrame = 0;
    char text[REMIX_HUD_TEXT_LENGTH] = {};
    float x = 0.0f;
    float y = 0.0f;
    uint32_t rgba = 0xFFFFFFFF;
    float height = 12.0f;
};

struct HudBar {
    bool active = false;
    int lastFrame = 0;
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float fraction = 0.0f;
    uint32_t rgba = 0xFFFFFFFF;
    float flash = 0.0f;
};

std::array<HudLabel, REMIX_HUD_TEXT_SLOTS> sLabels;
std::array<HudBar, REMIX_HUD_BAR_SLOTS> sBars;
int sCurrentFrame = 0;

/** Largest 4:3 rectangle centred in the viewport work area: the game image. */
void GameRect(const ImGuiViewport *viewport, ImVec2 *origin, float *scale) {
    const float scaleX = viewport->WorkSize.x / kN64ScreenWidth;
    const float scaleY = viewport->WorkSize.y / kN64ScreenHeight;
    *scale = std::min(scaleX, scaleY);
    origin->x = viewport->WorkPos.x + (viewport->WorkSize.x - kN64ScreenWidth * *scale) * 0.5f;
    origin->y = viewport->WorkPos.y + (viewport->WorkSize.y - kN64ScreenHeight * *scale) * 0.5f;
}

ImU32 ColorOf(uint32_t rgba) {
    return IM_COL32((rgba >> 24) & 0xFF, (rgba >> 16) & 0xFF, (rgba >> 8) & 0xFF, rgba & 0xFF);
}

/** Draws the label with the Remix font; the height is the glyph box height, so the font scale is height / 16. */
void DrawLabelWithRemixFont(ImDrawList *drawList, ImVec2 origin, float scale, const HudLabel &label) {
    const float fontScale = label.height / RemixFont::kGlyphHeight;
    const float textWidth = RemixFont::Measure(label.text, fontScale) * scale;
    const float boxHeight = label.height * scale;
    const ImVec2 position(origin.x + label.x * scale - textWidth * 0.5f, origin.y + label.y * scale - boxHeight * 0.5f);

    RemixFont::Draw(drawList, position, scale, fontScale, ColorOf(label.rgba), label.text);
}

void DrawLabel(ImDrawList *drawList, ImVec2 origin, float scale, const HudLabel &label) {
    if (RemixFont::Ensure()) {
        DrawLabelWithRemixFont(drawList, origin, scale, label);
        return;
    }
    const float fontSize = label.height * scale;
    const ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label.text);
    const ImVec2 position(origin.x + label.x * scale - textSize.x * 0.5f,
                          origin.y + label.y * scale - textSize.y * 0.5f);
    const float shadow = std::max(1.0f, fontSize * 0.08f);

    drawList->AddText(ImGui::GetFont(), fontSize, ImVec2(position.x + shadow, position.y + shadow), kShadowColor,
                      label.text);
    drawList->AddText(ImGui::GetFont(), fontSize, position, ColorOf(label.rgba), label.text);
}

void DrawBar(ImDrawList *drawList, ImVec2 origin, float scale, const HudBar &bar) {
    const ImVec2 min(origin.x + bar.x * scale, origin.y + bar.y * scale);
    const ImVec2 max(min.x + bar.width * scale, min.y + bar.height * scale);
    const ImVec2 fillMax(min.x + bar.width * scale * std::clamp(bar.fraction, 0.0f, 1.0f), max.y);

    drawList->AddRectFilled(min, max, IM_COL32(0, 0, 0, 160));
    drawList->AddRectFilled(min, fillMax, ColorOf(bar.rgba));
    if (bar.flash > 0.0f) {
        drawList->AddRectFilled(min, max, IM_COL32(255, 255, 255, static_cast<int>(255.0f * std::clamp(bar.flash, 0.0f, 1.0f))));
    }
    drawList->AddRect(min, max, IM_COL32(255, 255, 255, 220));
}

void DrawHudBars(ImDrawList *drawList, ImVec2 origin, float scale) {
    for (HudBar &bar : sBars) {
        if (!bar.active) {
            continue;
        }
        if (sCurrentFrame - bar.lastFrame > kLabelLifeFrames) {
            bar.active = false;
            continue;
        }
        DrawBar(drawList, origin, scale, bar);
    }
}

void DrawHudLabels() {
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImDrawList *drawList = ImGui::GetForegroundDrawList();
    ImVec2 origin;
    float scale;

    sCurrentFrame = ImGui::GetFrameCount();
    GameRect(viewport, &origin, &scale);
    DrawHudBars(drawList, origin, scale);
    for (HudLabel &label : sLabels) {
        if (!label.active) {
            continue;
        }
        if (sCurrentFrame - label.lastFrame > kLabelLifeFrames) {
            label.active = false;
            continue;
        }
        DrawLabel(drawList, origin, scale, label);
    }
}

class RemixHudTextWindow : public Ship::GuiWindow {
  public:
    RemixHudTextWindow() : Ship::GuiWindow("", true, "Remix HUD Text") {
    }

    /** Overrides GuiWindow::Draw, which opens an ImGui window. The overlay has no window of its own. */
    void Draw() override {
        DrawElement();
    }

    void DrawElement() override {
        DrawHudLabels();
    }

    void InitElement() override {
    }

    void UpdateElement() override {
    }
};

} // namespace

extern "C" void remix_hud_text_put(int slot, const char *text, float x, float y, uint32_t rgba, float height) {
    if (slot < 0 || slot >= REMIX_HUD_TEXT_SLOTS || text == nullptr) {
        return;
    }
    HudLabel &label = sLabels[slot];
    std::strncpy(label.text, text, REMIX_HUD_TEXT_LENGTH - 1);
    label.text[REMIX_HUD_TEXT_LENGTH - 1] = '\0';
    label.x = x;
    label.y = y;
    label.rgba = rgba;
    label.height = height;
    label.active = true;
    label.lastFrame = sCurrentFrame;
}

extern "C" void remix_hud_text_putf(int slot, float x, float y, uint32_t rgba, float height, const char *format, ...) {
    char text[REMIX_HUD_TEXT_LENGTH];
    va_list arguments;

    va_start(arguments, format);
    std::vsnprintf(text, sizeof(text), format, arguments);
    va_end(arguments);
    remix_hud_text_put(slot, text, x, y, rgba, height);
}

extern "C" void remix_hud_bar_put(int slot, float x, float y, float width, float height, float fraction, uint32_t rgba,
                                  float flash) {
    if (slot < 0 || slot >= REMIX_HUD_BAR_SLOTS) {
        return;
    }
    HudBar &bar = sBars[slot];
    bar.x = x;
    bar.y = y;
    bar.width = width;
    bar.height = height;
    bar.fraction = fraction;
    bar.rgba = rgba;
    bar.flash = flash;
    bar.active = true;
    bar.lastFrame = sCurrentFrame;
}

extern "C" void remix_hud_text_hide(int slot) {
    if (slot < 0 || slot >= REMIX_HUD_TEXT_SLOTS) {
        return;
    }
    sLabels[slot].active = false;
}

extern "C" void remix_hud_text_clear(void) {
    for (HudLabel &label : sLabels) {
        label.active = false;
    }
    for (HudBar &bar : sBars) {
        bar.active = false;
    }
}

std::shared_ptr<Ship::GuiWindow> remix_hud_text_make_window() {
    return std::make_shared<RemixHudTextWindow>();
}

#endif
