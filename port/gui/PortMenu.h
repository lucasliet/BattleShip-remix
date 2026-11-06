/*
 * SPDX-License-Identifier: MIT
 *
 * Portions of this file are derived from the Ship of Harkinian (SoH) project
 *   Copyright (c) The Harbour Masters
 *   https://github.com/HarbourMasters/Shipwright
 * Licensed under the MIT License; see LICENSE at repository root.
 */

#pragma once

#include "Menu.h"
#include "MenuTypes.h"

#include <map>

#ifdef REGION_REMIX
#include "remix/remix_toggles.h"
#endif

namespace ssb64 {

class PortMenu : public Ship::Menu {
  public:
    PortMenu();
    ~PortMenu() override = default;

    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override;
    void Draw() override;
    void NotifyUpdateOnce();
#ifdef REGION_REMIX
    /**
     * Adds the Remix section after Settings. The menu is built when the window opens, before the game archive and
     * the Remix toggles load, so PortInit calls this once remix_toggles_init and remix_panel_init have run.
     */
    void AddRemixSection();
#endif

  protected:
    void AddSidebarEntry(std::string sectionName, std::string sidbarName, uint32_t columnCount);
    WidgetInfo& AddWidget(WidgetPath& pathInfo, std::string widgetName, WidgetType widgetType);
    void AddMenuElements();
    void AddMenuSettings();
    void AddMenuAssets();
    void AddMenuAbout();
#ifdef REGION_REMIX
    void AddMenuRemix();
    void AddMenuRemixGroup(WidgetPath& path, remix_toggles::Group group);
    void AddMenuRemixToggle(WidgetPath& path, const remix_toggles::Toggle& toggle);
    void AddMenuRemixToggleAll(WidgetPath& path, int& shown, const remix_toggles::ToggleAllRow& row);
    void AddMenuRemixSubProfiles(WidgetPath& path, int& shown, const remix_toggles::Toggle& loader);
    void AddMenuRemixPlayerPanel();
    void AddMenuRemixOtherScreens();
    void AddMenuRemixModes();
    void AddMenuRemixTraining();
#endif
#ifndef DISABLE_SCRIPTING
    void AddMenuMods();
#endif

  private:
    bool mShowReextractMessage = false;
    bool mMenuElementsInitialized = false;
#ifdef REGION_REMIX
    bool mWasVisible = false;
    bool mUpdateNoticeShown = false;
#endif
};

} // namespace ssb64
