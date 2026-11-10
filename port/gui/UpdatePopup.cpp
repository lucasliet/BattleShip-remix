#include "UpdatePopup.h"

#include "enhancements/enhancements.h"

#include <imgui.h>
#include <ship/Context.h>
#include <ship/controller/controldeck/ControlDeck.h>
#include <ship/window/Window.h>
#include <ship/window/gui/Gui.h>

#include <string>

namespace ssb64 {

namespace {

constexpr int32_t kUpdatePopupInputBlockId = 0x5550;

/* The Switch SDL maps buttons by position: Nintendo A (right face button) arrives as GamepadFaceRight and
 * Nintendo B (bottom) as GamepadFaceDown, the reverse of the Xbox layout. Pressing B on a docked Switch
 * started the update before this swap. */
#ifdef __SWITCH__
constexpr ImGuiKey kConfirmKey = ImGuiKey_GamepadFaceRight;
constexpr ImGuiKey kCancelKey = ImGuiKey_GamepadFaceDown;
constexpr const char* kCloseGameLabel = "Close game (+)";
#else
constexpr ImGuiKey kConfirmKey = ImGuiKey_GamepadFaceDown;
constexpr ImGuiKey kCancelKey = ImGuiKey_GamepadFaceRight;
constexpr const char* kCloseGameLabel = "Close game (Start)";
#endif

enum class UpdateStage { Checking, Offered, Downloading, Finished, Dismissed };

class UpdatePopupWindow : public Ship::GuiWindow {
  public:
    UpdatePopupWindow() : Ship::GuiWindow("", true, "Update Popup") {
    }

    /** Overrides GuiWindow::Draw, which opens an ImGui window; the popup opens its own. */
    void Draw() override {
        DrawElement();
    }

    void InitElement() override {
    }

    void UpdateElement() override {
    }

    void DrawElement() override {
        if (mStage == UpdateStage::Dismissed) return;
        if (mStage == UpdateStage::Checking && !AdvanceFromChecking()) return;
        if (mStage == UpdateStage::Downloading) AdvanceFromDownloading();

        CaptureInput();
        BeginCenteredWindow();
        if (mStage == UpdateStage::Offered) DrawOffer();
        else if (mStage == UpdateStage::Downloading) ImGui::TextUnformatted(enhancements::GetDownloadStatus().c_str());
        else DrawResult();
        ImGui::End();
        mFocusPending = false;
    }

  private:
    UpdateStage mStage = UpdateStage::Checking;
    bool mCheckStarted = false;
    bool mFocusPending = true;
    bool mRestartRequested = false;

    /**
     * Starts the release check once and moves on when it ends.
     * @return true when the popup has something to show
     */
    bool AdvanceFromChecking() {
        if (!mCheckStarted) {
            mCheckStarted = true;
            enhancements::CheckForUpdatesAsync(false);
        }
        if (enhancements::IsCheckingForUpdates()) return false;
        if (!enhancements::IsUpdateAvailable()) {
            mStage = UpdateStage::Dismissed;
            return false;
        }
        mStage = UpdateStage::Offered;
        return true;
    }

    void AdvanceFromDownloading() {
        if (enhancements::IsDownloading()) return;
        mStage = UpdateStage::Finished;
        if (enhancements::IsDownloadComplete() && enhancements::CanRestartAfterUpdate() && !mRestartRequested) {
            mRestartRequested = true;
            enhancements::RestartAfterUpdate();
        }
    }

    /** Keeps the game from reading the pad and lets the pad drive the popup's buttons. */
    void CaptureInput() {
        Ship::Context::GetInstance()->GetControlDeck()->BlockGameInput(kUpdatePopupInputBlockId);
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    }

    void ReleaseInput() {
        auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
        Ship::Context::GetInstance()->GetControlDeck()->UnblockGameInput(kUpdatePopupInputBlockId);
        if (!gui->GetMenuOrMenubarVisible()) {
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
        }
        mStage = UpdateStage::Dismissed;
    }

    void BeginCenteredWindow() {
        const ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        if (mFocusPending) ImGui::SetNextWindowFocus();
        ImGui::Begin("Update available", nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
    }

    void DrawVersionLine() {
        ImGui::Text("BattleShip Remix %s is available (installed: %s).", enhancements::GetLatestVersion().c_str(),
                    BATTLESHIP_CURRENT_VERSION);
    }

    void DrawOffer() {
        if (!enhancements::CanInstallUpdateInGame()) {
            DrawReleasePageOffer();
            return;
        }
        DrawVersionLine();
        ImGui::TextUnformatted("Download and install it now? The game restarts when it finishes.");
        if (mFocusPending) ImGui::SetKeyboardFocusHere();
        if (ImGui::Button("Update (A)") || PadPressed(kConfirmKey)) {
            enhancements::StartGameUpdate();
            mStage = UpdateStage::Downloading;
            return;
        }
        ImGui::SameLine();
        if (ImGui::Button("Later (B)") || PadPressed(kCancelKey)) ReleaseInput();
    }

    /** Offer for platforms that do not install updates themselves (Windows): open the release page. */
    void DrawReleasePageOffer() {
        DrawVersionLine();
        ImGui::TextUnformatted("Download the zip from the release page and extract it into the game folder,");
        ImGui::TextUnformatted("replacing the old files.");
        if (mFocusPending) ImGui::SetKeyboardFocusHere();
        if (ImGui::Button("Open release page (A)") || PadPressed(kConfirmKey)) {
            enhancements::StartGameUpdate();
            ReleaseInput();
            return;
        }
        ImGui::SameLine();
        if (ImGui::Button("Later (B)") || PadPressed(kCancelKey)) ReleaseInput();
    }

    /**
     * Reads a pad button straight from ImGui, so the popup answers the pad even when ImGui's gamepad
     * navigation has no focused item (docked Switch: the touch screen is out of reach).
     * @return true on the frame the button goes down, never on the frame the popup first appears
     */
    bool PadPressed(ImGuiKey key) {
        return !mFocusPending && ImGui::IsKeyPressed(key, false);
    }

    void DrawResult() {
        const bool needsManualInstall = enhancements::IsDownloadComplete() && !mRestartRequested;
        if (needsManualInstall) {
            ImGui::TextUnformatted("Update downloaded. Finish the install, then reopen the game.");
        } else {
            ImGui::TextUnformatted(enhancements::GetDownloadStatus().c_str());
        }
        if (mFocusPending) ImGui::SetKeyboardFocusHere();
        if (needsManualInstall) {
            if (ImGui::Button(kCloseGameLabel) || PadPressed(ImGuiKey_GamepadStart)) {
                Ship::Context::GetInstance()->GetWindow()->Close();
            }
            ImGui::SameLine();
        }
        if (ImGui::Button("Continue (A)") || PadPressed(kConfirmKey)) ReleaseInput();
    }
};

} // namespace

std::shared_ptr<Ship::GuiWindow> MakeUpdatePopupWindow() {
    return std::make_shared<UpdatePopupWindow>();
}

} // namespace ssb64
