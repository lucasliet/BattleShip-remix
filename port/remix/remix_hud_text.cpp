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

#include <algorithm>
#include <array>
#include <cfloat>
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

std::array<HudLabel, REMIX_HUD_TEXT_SLOTS> sLabels;
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

void DrawLabel(ImDrawList *drawList, ImVec2 origin, float scale, const HudLabel &label) {
    const float fontSize = label.height * scale;
    const ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label.text);
    const ImVec2 position(origin.x + label.x * scale - textSize.x * 0.5f,
                          origin.y + label.y * scale - textSize.y * 0.5f);
    const float shadow = std::max(1.0f, fontSize * 0.08f);

    drawList->AddText(ImGui::GetFont(), fontSize, ImVec2(position.x + shadow, position.y + shadow), kShadowColor,
                      label.text);
    drawList->AddText(ImGui::GetFont(), fontSize, position, ColorOf(label.rgba), label.text);
}

void DrawHudLabels() {
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImDrawList *drawList = ImGui::GetForegroundDrawList();
    ImVec2 origin;
    float scale;

    sCurrentFrame = ImGui::GetFrameCount();
    GameRect(viewport, &origin, &scale);
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
}

std::shared_ptr<Ship::GuiWindow> remix_hud_text_make_window() {
    return std::make_shared<RemixHudTextWindow>();
}

#endif
