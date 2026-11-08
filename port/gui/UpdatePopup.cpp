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

    void DrawOffer() {
        ImGui::Text("BattleShip Remix %s is available (installed: %s).", enhancements::GetLatestVersion().c_str(),
                    BATTLESHIP_CURRENT_VERSION);
#if defined(__APPLE__)
        ImGui::TextUnformatted("Download it now? The disk image opens and you copy the app to Applications.");
#else
        ImGui::TextUnformatted("Download and install it now? The game restarts when it finishes.");
#endif
        if (mFocusPending) ImGui::SetKeyboardFocusHere();
        if (ImGui::Button("Update")) {
            enhancements::StartGameUpdate();
            mStage = UpdateStage::Downloading;
        }
        ImGui::SameLine();
        if (ImGui::Button("Later")) ReleaseInput();
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
            if (ImGui::Button("Close game")) Ship::Context::GetInstance()->GetWindow()->Close();
            ImGui::SameLine();
        }
        if (ImGui::Button("Continue")) ReleaseInput();
    }
};

} // namespace

std::shared_ptr<Ship::GuiWindow> MakeUpdatePopupWindow() {
    return std::make_shared<UpdatePopupWindow>();
}

} // namespace ssb64
