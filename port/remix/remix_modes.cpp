/**
 * remix_modes.cpp - see remix_modes.h.
 */

#include "remix_modes.h"

#include <libultraship/bridge/consolevariablebridge.h>

#include <map>
#include <string>
#include <vector>

#include "remix_css_data.h"
#include "remix_roster_data.h"

namespace {

std::string TagTeamCVar(int port, int slot) {
    return "gRemix.Modes.TagTeam.P" + std::to_string(port + 1) + ".S" + std::to_string(slot + 1);
}

}  // namespace

std::vector<std::string> remix_modes_tagteam_cvars() {
    std::vector<std::string> names;
    for (int port = 0; port < 4; port++) {
        for (int slot = 1; slot < REMIX_MODES_TAGTEAM_SLOTS; slot++) {
            names.push_back(TagTeamCVar(port, slot));
        }
    }
    return names;
}

extern "C" const char *remix_modes_fighter_name(int fkind) {
    for (const RemixRosterEntry &entry : kRemixRoster) {
        if (entry.remix_id == fkind) {
            return entry.name;
        }
    }
    return nullptr;
}

std::map<int32_t, const char *> remix_modes_tagteam_fighters() {
    std::map<int32_t, const char *> fighters = { { 0, "Random" } };
    for (int i = 0; i < REMIX_CSS_NUM_SLOTS; i++) {
        const char *name = remix_modes_fighter_name(kRemixCssSlotIds[i]);
        if (name != nullptr) {
            fighters[i + 1] = name;
        }
    }
    return fighters;
}

extern "C" int remix_modes_tagteam_slot(int port, int slot) {
    if (port < 0 || port >= 4 || slot < 1 || slot >= REMIX_MODES_TAGTEAM_SLOTS) {
        return -1;
    }
    const int choice = CVarGetInteger(TagTeamCVar(port, slot).c_str(), 0);
    if (choice <= 0 || choice > REMIX_CSS_NUM_SLOTS) {
        return -1;
    }
    return kRemixCssSlotIds[choice - 1];
}

extern "C" int remix_modes_smashketball_type(void) {
    return CVarGetInteger(REMIX_MODES_CVAR_SMASHKETBALL_TYPE, 0) != 0 ? 1 : 0;
}

extern "C" void remix_modes_smashketball_set_type(int type) {
    CVarSetInteger(REMIX_MODES_CVAR_SMASHKETBALL_TYPE, type != 0 ? 1 : 0);
}

extern "C" int remix_modes_tagteam_set_slot(int port, int slot, int fkind) {
    if (port < 0 || port >= 4 || slot < 1 || slot >= REMIX_MODES_TAGTEAM_SLOTS) {
        return -1;
    }
    int choice = 0;
    for (int i = 0; i < REMIX_CSS_NUM_SLOTS; i++) {
        if (kRemixCssSlotIds[i] == fkind) {
            choice = i + 1;
            break;
        }
    }
    CVarSetInteger(TagTeamCVar(port, slot).c_str(), choice);
    return choice > 0 ? kRemixCssSlotIds[choice - 1] : -1;
}
