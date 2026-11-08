#ifndef SSB64_GUI_UPDATE_POPUP_H
#define SSB64_GUI_UPDATE_POPUP_H

#include <memory>

#include <ship/window/gui/GuiWindow.h>

namespace ssb64 {

/**
 * Builds the start-up update popup: it checks the latest release, offers it, downloads and installs it on accept
 * and restarts the game when the platform can. Any refusal or failure continues to the game.
 * @return the window, to be added to the Gui
 */
std::shared_ptr<Ship::GuiWindow> MakeUpdatePopupWindow();

} // namespace ssb64

#endif
