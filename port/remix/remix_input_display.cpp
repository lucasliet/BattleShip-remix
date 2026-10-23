/**
 * remix_input_display.cpp - see remix_input_display.h.
 *
 * The overlay is a GuiWindow without an ImGui window of its own: it draws on the foreground draw list, so it sits above
 * the game image and the menus. Button bits are the libultra CONT_* values of decomp/include/PR/os.h.
 */

#include "remix_input_display.h"

#ifdef REGION_REMIX

#include "remix/remix_player_panel.h"

#include <imgui.h>
#include <libultraship/libultraship.h>

#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <memory>
#include <vector>

namespace {

constexpr int kInputDisplayPorts = REMIX_PANEL_PORTS;
constexpr float kStickRangeMax = 80.0f;
constexpr float kControllerWidth = 180.0f;
constexpr float kControllerHeight = 110.0f;
constexpr float kControllerGap = 12.0f;
constexpr float kScreenMargin = 10.0f;
constexpr float kReferenceScreenHeight = 720.0f;
constexpr float kScaleMin = 0.6f;
constexpr float kScaleMax = 2.0f;
constexpr float kTintAlpha = 90.0f / 255.0f;
constexpr float kStickDotTravel = 14.0f;

constexpr uint16_t kButtonA = 0x8000;
constexpr uint16_t kButtonB = 0x4000;
constexpr uint16_t kButtonZ = 0x2000;
constexpr uint16_t kButtonStart = 0x1000;
constexpr uint16_t kButtonDpadUp = 0x0800;
constexpr uint16_t kButtonDpadDown = 0x0400;
constexpr uint16_t kButtonDpadLeft = 0x0200;
constexpr uint16_t kButtonDpadRight = 0x0100;
constexpr uint16_t kButtonL = 0x0020;
constexpr uint16_t kButtonR = 0x0010;
constexpr uint16_t kButtonCUp = 0x0008;
constexpr uint16_t kButtonCDown = 0x0004;
constexpr uint16_t kButtonCLeft = 0x0002;
constexpr uint16_t kButtonCRight = 0x0001;

constexpr uint32_t kLitColor = IM_COL32(255, 255, 255, 255);

/* Port colours of the N64 controller ports 1 to 4 (red, blue, yellow, green), the tint of each outline. */
constexpr std::array<ImVec4, kInputDisplayPorts> kPortTints = {
    ImVec4(0.95f, 0.25f, 0.25f, 1.0f),
    ImVec4(0.30f, 0.50f, 1.00f, 1.0f),
    ImVec4(1.00f, 0.85f, 0.20f, 1.0f),
    ImVec4(0.30f, 0.85f, 0.35f, 1.0f),
};

enum class InputDisplayAnchor { None, Top, Bottom };

struct InputDisplaySample {
    bool active = false;
    uint16_t buttons = 0;
    int8_t stickX = 0;
    int8_t stickY = 0;
};

std::array<InputDisplaySample, kInputDisplayPorts> sSamples;

/** Anchor of `port` from its Input Display panel value: 0 is disabled, 1 top, 2 bottom. */
InputDisplayAnchor AnchorOf(int port) {
    switch (remix_panel_get(REMIX_PANEL_INPUT_DISPLAY, port)) {
    case 1:
        return InputDisplayAnchor::Top;
    case 2:
        return InputDisplayAnchor::Bottom;
    default:
        return InputDisplayAnchor::None;
    }
}

ImVec2 Point(ImVec2 origin, float scale, float x, float y) {
    return ImVec2(origin.x + x * scale, origin.y + y * scale);
}

ImU32 TintColor(int port) {
    const ImVec4 &tint = kPortTints[port];
    return IM_COL32(static_cast<int>(tint.x * 255.0f), static_cast<int>(tint.y * 255.0f),
                    static_cast<int>(tint.z * 255.0f), static_cast<int>(kTintAlpha * 255.0f));
}

ImU32 OutlineColor(int port) {
    const ImVec4 &tint = kPortTints[port];
    return ImGui::GetColorU32(tint);
}

ImU32 ButtonColor(int port, bool pressed) {
    return pressed ? kLitColor : TintColor(port);
}

void DrawBody(ImDrawList *drawList, ImVec2 origin, float scale, int port) {
    const ImVec2 min = Point(origin, scale, 0.0f, 0.0f);
    const ImVec2 max = Point(origin, scale, kControllerWidth, kControllerHeight);
    drawList->AddRectFilled(min, max, TintColor(port), 24.0f * scale);
    drawList->AddRect(min, max, OutlineColor(port), 24.0f * scale, 0, 2.0f * scale);
}

void DrawTriggers(ImDrawList *drawList, ImVec2 origin, float scale, int port, uint16_t buttons) {
    drawList->AddRectFilled(Point(origin, scale, 18.0f, 0.0f), Point(origin, scale, 62.0f, 14.0f),
                            ButtonColor(port, (buttons & kButtonL) != 0), 4.0f * scale);
    drawList->AddRectFilled(Point(origin, scale, 118.0f, 0.0f), Point(origin, scale, 162.0f, 14.0f),
                            ButtonColor(port, (buttons & kButtonR) != 0), 4.0f * scale);
}

void DrawStick(ImDrawList *drawList, ImVec2 origin, float scale, const InputDisplaySample &sample, int port) {
    const ImVec2 center = Point(origin, scale, 56.0f, 46.0f);
    const float clampedX = std::clamp(static_cast<float>(sample.stickX), -kStickRangeMax, kStickRangeMax);
    const float clampedY = std::clamp(static_cast<float>(sample.stickY), -kStickRangeMax, kStickRangeMax);
    const ImVec2 dot = ImVec2(center.x + clampedX / kStickRangeMax * kStickDotTravel * scale,
                              center.y - clampedY / kStickRangeMax * kStickDotTravel * scale);

    drawList->AddCircle(center, 18.0f * scale, OutlineColor(port), 32, 2.0f * scale);
    drawList->AddCircleFilled(dot, 5.0f * scale, kLitColor, 16);
}

void DrawDpadArm(ImDrawList *drawList, ImVec2 origin, float scale, float x0, float y0, float x1, float y1,
                 ImU32 color) {
    drawList->AddRectFilled(Point(origin, scale, x0, y0), Point(origin, scale, x1, y1), color, 2.0f * scale);
}

void DrawDpad(ImDrawList *drawList, ImVec2 origin, float scale, int port, uint16_t buttons) {
    const float cx = 42.0f;
    const float cy = 88.0f;
    DrawDpadArm(drawList, origin, scale, cx - 4.0f, cy - 12.0f, cx + 4.0f, cy - 4.0f,
                ButtonColor(port, (buttons & kButtonDpadUp) != 0));
    DrawDpadArm(drawList, origin, scale, cx - 4.0f, cy + 4.0f, cx + 4.0f, cy + 12.0f,
                ButtonColor(port, (buttons & kButtonDpadDown) != 0));
    DrawDpadArm(drawList, origin, scale, cx - 12.0f, cy - 4.0f, cx - 4.0f, cy + 4.0f,
                ButtonColor(port, (buttons & kButtonDpadLeft) != 0));
    DrawDpadArm(drawList, origin, scale, cx + 4.0f, cy - 4.0f, cx + 12.0f, cy + 4.0f,
                ButtonColor(port, (buttons & kButtonDpadRight) != 0));
}

void DrawCircleButton(ImDrawList *drawList, ImVec2 origin, float scale, float x, float y, float radius, int port,
                      bool pressed, const char *label) {
    const ImVec2 center = Point(origin, scale, x, y);
    drawList->AddCircleFilled(center, radius * scale, ButtonColor(port, pressed), 24);
    drawList->AddCircle(center, radius * scale, OutlineColor(port), 24, 1.5f * scale);
    const float fontSize = std::max(8.0f, radius * scale * 1.3f);
    const ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label);
    const ImU32 textColor = pressed ? IM_COL32(0, 0, 0, 255) : OutlineColor(port);
    drawList->AddText(ImGui::GetFont(), fontSize,
                      ImVec2(center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f), textColor, label);
}

void DrawFaceButtons(ImDrawList *drawList, ImVec2 origin, float scale, int port, uint16_t buttons) {
    DrawCircleButton(drawList, origin, scale, 116.0f, 74.0f, 7.0f, port, (buttons & kButtonB) != 0, "B");
    DrawCircleButton(drawList, origin, scale, 142.0f, 80.0f, 10.0f, port, (buttons & kButtonA) != 0, "A");
    DrawCircleButton(drawList, origin, scale, 90.0f, 60.0f, 6.0f, port, (buttons & kButtonStart) != 0, "S");
}

void DrawCButtons(ImDrawList *drawList, ImVec2 origin, float scale, int port, uint16_t buttons) {
    DrawCircleButton(drawList, origin, scale, 138.0f, 22.0f, 5.0f, port, (buttons & kButtonCUp) != 0, "");
    DrawCircleButton(drawList, origin, scale, 138.0f, 50.0f, 5.0f, port, (buttons & kButtonCDown) != 0, "");
    DrawCircleButton(drawList, origin, scale, 124.0f, 36.0f, 5.0f, port, (buttons & kButtonCLeft) != 0, "");
    DrawCircleButton(drawList, origin, scale, 152.0f, 36.0f, 5.0f, port, (buttons & kButtonCRight) != 0, "");
}

void DrawZTrigger(ImDrawList *drawList, ImVec2 origin, float scale, int port, uint16_t buttons) {
    drawList->AddRectFilled(Point(origin, scale, 77.0f, 87.0f), Point(origin, scale, 99.0f, 97.0f),
                            ButtonColor(port, (buttons & kButtonZ) != 0), 3.0f * scale);
}

/** Draws one controller outline with its stick, D-pad and buttons. Shapes are lit while their button is held. */
void DrawController(ImDrawList *drawList, ImVec2 origin, float scale, int port, const InputDisplaySample &sample) {
    DrawBody(drawList, origin, scale, port);
    DrawTriggers(drawList, origin, scale, port, sample.buttons);
    DrawStick(drawList, origin, scale, sample, port);
    DrawDpad(drawList, origin, scale, port, sample.buttons);
    DrawFaceButtons(drawList, origin, scale, port, sample.buttons);
    DrawCButtons(drawList, origin, scale, port, sample.buttons);
    DrawZTrigger(drawList, origin, scale, port, sample.buttons);
}

/** Scale of the outlines: proportional to the window height, clamped so they stay readable and do not cover the game. */
float ScaleForViewport(const ImGuiViewport *viewport) {
    const float scale = viewport->WorkSize.y / kReferenceScreenHeight;
    return std::clamp(scale, kScaleMin, kScaleMax);
}

/** Draws every port anchored to one edge, side by side and centred, in port order. */
void DrawRow(ImDrawList *drawList, const ImGuiViewport *viewport, InputDisplayAnchor anchor) {
    std::vector<int> ports;
    for (int port = 0; port < kInputDisplayPorts; port++) {
        if (sSamples[port].active && AnchorOf(port) == anchor) {
            ports.push_back(port);
        }
    }
    if (ports.empty()) {
        return;
    }

    const float scale = ScaleForViewport(viewport);
    const float slotWidth = kControllerWidth * scale;
    const float rowWidth = slotWidth * ports.size() + kControllerGap * scale * (ports.size() - 1);
    const float rowX = viewport->WorkPos.x + (viewport->WorkSize.x - rowWidth) * 0.5f;
    const float rowY = (anchor == InputDisplayAnchor::Top)
                           ? viewport->WorkPos.y + kScreenMargin * scale
                           : viewport->WorkPos.y + viewport->WorkSize.y - (kControllerHeight + kScreenMargin) * scale;

    for (size_t slot = 0; slot < ports.size(); slot++) {
        const int port = ports[slot];
        const ImVec2 origin(rowX + slot * (slotWidth + kControllerGap * scale), rowY);
        DrawController(drawList, origin, scale, port, sSamples[port]);
    }
}

/** Frame draw of the overlay: both edges, drawn on the foreground list above the game image and the menus. */
void DrawInputDisplays() {
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImDrawList *drawList = ImGui::GetForegroundDrawList();
    DrawRow(drawList, viewport, InputDisplayAnchor::Top);
    DrawRow(drawList, viewport, InputDisplayAnchor::Bottom);
}

class RemixInputDisplayWindow : public Ship::GuiWindow {
  public:
    RemixInputDisplayWindow() : Ship::GuiWindow("", true, "Remix Input Display") {
    }

    /** Overrides GuiWindow::Draw, which opens an ImGui window. The overlay has no window of its own. */
    void Draw() override {
        DrawElement();
    }

    void DrawElement() override {
        DrawInputDisplays();
    }

    void InitElement() override {
    }

    void UpdateElement() override {
    }
};

} // namespace

extern "C" void remix_input_display_update(int port, uint16_t buttons, int8_t stick_x, int8_t stick_y) {
    if (port < 0 || port >= kInputDisplayPorts) {
        return;
    }
    sSamples[port] = InputDisplaySample{ true, buttons, stick_x, stick_y };
}

extern "C" void remix_input_display_clear(void) {
    for (InputDisplaySample &sample : sSamples) {
        sample = InputDisplaySample{};
    }
}

std::shared_ptr<Ship::GuiWindow> remix_input_display_make_window() {
    return std::make_shared<RemixInputDisplayWindow>();
}

#endif
