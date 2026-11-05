/**
 * remix_player_panel.cpp - see remix_player_panel.h.
 */

#include "remix_player_panel.h"

#include <libultraship/bridge/consolevariablebridge.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <string>

extern "C" {
#include "port_log.h"
}

namespace remix_panel {
namespace {

const std::vector<std::string> kFighterNames = {
    "Mario", "Fox", "DK", "Samus", "Luigi", "Link", "Yoshi", "Captain Falcon", "Pikachu", "Jigglypuff", "Ness",
    "Falco", "Ganondorf", "Young Link", "Dr. Mario", "Wario", "Bowser", "Wolf", "Conker", "Mewtwo", "Marth", "Sonic",
    "Sheik", "Marina", "Dedede", "Goemon", "Banjo", "Crash", "Peach", "Dark Samus", "Lucas", "Peppy", "Slippy", "Roy",
    "Dr. Luigi", "Lanky Kong", "Dragon King", "Ebisumaru", "Piano", "Super Sonic",
};

const std::vector<std::string> kItemNames = {
    "None", "Beam Sword", "HomeRun Bat", "Fan", "Star Rod", "Ray Gun", "Fire Flower", "Hammer", "MS Bomb", "Bob-omb",
    "Bumper", "Green Shell", "Red Shell", "Poke Ball", "Spiny Shell", "Deku Nut", "Pitfall", "Golden Gun", "Mr. Saturn",
    "Random",
};

const std::vector<std::string> kHandicapNames = {
    "Default", "1", "2", "3", "4", "5", "6", "7", "8", "9",
    "Yoshi V. Easy", "Yoshi Easy", "Yoshi Normal", "Yoshi Hard", "Yoshi V. Hard",
    "Kirby V. Easy", "Kirby Easy", "Kirby Normal", "Kirby Hard", "Kirby V. Hard",
    "Polygon V. Easy", "Polygon Easy", "Polygon Normal", "Polygon Hard", "Polygon V. Hard",
    "G. DK V. Easy", "G. DK Easy", "G. DK Normal", "G. DK Hard", "G. DK V. Hard",
    "M. Mario V. Easy", "M. Mario Easy", "M. Mario Normal", "M. Mario Hard", "M. Mario V. Hard",
    "MH V. Easy", "MH Easy", "MH Normal", "MH Hard", "MH V. Hard",
    "Samus",
};

std::vector<std::string> WithPrefix(const std::vector<std::string> &head, const std::vector<std::string> &tail,
                                    const std::vector<std::string> &last) {
    std::vector<std::string> all = head;
    all.insert(all.end(), tail.begin(), tail.end());
    all.insert(all.end(), last.begin(), last.end());
    return all;
}

std::vector<std::string> PracticeStages() {
    std::vector<std::string> stages = { "OFF" };
    for (int stage = 1; stage <= 10; stage++) {
        stages.push_back("Stage " + std::to_string(stage));
    }
    stages.push_back("Final Stage");
    return stages;
}

const std::vector<Option> kOptions = {
    { REMIX_PANEL_SHIELD, "Shield", "Shield", 0, 17, 0,
      { "Default", "Red", "Orange", "Yellow", "Lime", "Green", "Turquoise", "Cyan", "Azure", "Blue", "Purple", "Magenta",
        "Pink", "Brown", "Black", "White", "Vanilla", "Costume" } },
    { REMIX_PANEL_PLAYER_TAG, "PlayerTag", "Player Tag (0 = default)", 0, 20, 0, {} },
    { REMIX_PANEL_VISIBILITY, "Visibility", "Visibility", 0, 3, 0, { "Default", "Cloaked", "None", "Dark" } },
    { REMIX_PANEL_SKELETON, "Skeleton", "Skeleton", 0, 1, 0, { "Disabled", "Enabled" } },
    { REMIX_PANEL_MODEL_DISPLAY, "ModelDisplay", "Model", 0, 2, 0, { "Default", "Hi Poly", "Lo Poly" } },
    { REMIX_PANEL_INPUT_DISPLAY, "InputDisplay", "Input HUD", 0, 2, 0, { "Disabled", "Top", "Bottom" } },
    { REMIX_PANEL_SIZE, "Size", "Size", 0, 2, 0, { "Default", "Giant", "Tiny" } },
    { REMIX_PANEL_STOCK_MODE, "StockMode", "Stock", 0, 2, 0, { "Default", "Last", "Manual" } },
    { REMIX_PANEL_KNOCKBACK, "Knockback", "Knockback", 0, 1, 0, { "Default", "Random" } },
    { REMIX_PANEL_INPUT_DELAY, "InputDelay", "Input Delay (frames)", 0, 12, 0, {} },
    { REMIX_PANEL_HANDICAP, "Handicap", "CPU Handicap", 0, 40, 0, kHandicapNames },
    { REMIX_PANEL_START_WITH, "StartWith", "Start With", 0, 19, 0, kItemNames },
    { REMIX_PANEL_TAUNT_ITEM, "TauntItem", "Taunt Item", 0, 19, 0, kItemNames },
    { REMIX_PANEL_TAUNT_BUTTON, "TauntButton", "Taunt Button", 0, 8, 0,
      { "L", "C-Up", "C-Down", "C-Left", "C-Right", "D-Up", "D-Down", "D-Left", "D-Right" } },
    { REMIX_PANEL_KIRBY_HAT, "KirbyHat", "Kirby Hat", 0, 42, 0, WithPrefix({ "None" }, kFighterNames, { "???", "Random" }) },
    { REMIX_PANEL_DPAD_FUNCTIONS, "DpadFunctions", "D-Pad Map", 0, 3, 0, { "Off", "Smash", "Tilt", "Special" } },
    { REMIX_PANEL_DPAD_CONTROL, "DpadControl", "D-Pad Control", 0, 4, 0,
      { "Off", "Stick Swap", "Stickless", "Stick Swap J", "Stickless J" } },
    { REMIX_PANEL_DAMAGE, "Damage", "Initial Damage (%)", 0, 999, 0, {} },
    { REMIX_PANEL_POISON_DAMAGE, "PoisonDmg", "Poison Damage", 0, 4, 0, { "None", "Low", "Medium", "High", "Heal" } },
    { REMIX_PANEL_PRACTICE_1P, "Practice1P", "1P Practice", 0, 11, 0, PracticeStages() },
    { REMIX_PANEL_STOCK_COUNT, "StockCount", "Stock Count (Manual)", 0, 99, 3, {} },
};

std::array<std::string, REMIX_PANEL_TAG_COUNT> sTagNameOverrides;
std::array<bool, REMIX_PANEL_TAG_COUNT> sIsTagNameOverridden;
std::array<std::string, REMIX_PANEL_TAG_COUNT> sTagNameBuffers;
std::array<std::array<int, REMIX_PANEL_PORTS>, REMIX_PANEL_COUNT> sOverrides;
std::array<std::array<bool, REMIX_PANEL_PORTS>, REMIX_PANEL_COUNT> sIsOverridden;

bool IsValid(int option, int port) {
    return (option >= 0) && (option < REMIX_PANEL_COUNT) && (port >= 0) && (port < REMIX_PANEL_PORTS);
}

int Clamp(const Option &option, int value) {
    return std::max(option.min, std::min(option.max, value));
}

std::string Lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return std::tolower(c); });
    return text;
}

void ApplyOverride(const std::string &item) {
    const size_t dot = item.find('.');
    const size_t equals = item.find('=');
    if ((dot == std::string::npos) || (equals == std::string::npos) || (equals < dot)) {
        spdlog::warn("remix_panel: SSB64_REMIX_PANEL item '{}' is not <Key>.<port>=<value>", item);
        return;
    }
    const std::string key = Lower(item.substr(0, dot));
    const int port = std::atoi(item.substr(dot + 1, equals - dot - 1).c_str()) - 1;
    const int value = std::atoi(item.c_str() + equals + 1);
    for (const Option &option : kOptions) {
        if ((Lower(option.key) == key) && IsValid(option.id, port)) {
            sOverrides[option.id][port] = Clamp(option, value);
            sIsOverridden[option.id][port] = true;
            return;
        }
    }
    spdlog::warn("remix_panel: SSB64_REMIX_PANEL item '{}' names no option or a port outside 1..4", item);
}

/** Applies one "<tag 1..20>=<name>" item of SSB64_REMIX_TAG_NAMES. */
void ApplyTagNameOverride(const std::string &item) {
    const size_t equals = item.find('=');
    const int tag = (equals == std::string::npos) ? 0 : std::atoi(item.substr(0, equals).c_str());
    if ((tag < 1) || (tag > REMIX_PANEL_TAG_COUNT)) {
        spdlog::warn("remix_panel: SSB64_REMIX_TAG_NAMES item '{}' is not <tag 1..{}>=<name>", item, REMIX_PANEL_TAG_COUNT);
        return;
    }
    sTagNameOverrides[tag - 1] = item.substr(equals + 1);
    sIsTagNameOverridden[tag - 1] = true;
}

/** Splits a ';' list and gives each item to apply. */
void ForEachListItem(const std::string &list, void (*apply)(const std::string &)) {
    size_t start = 0;
    while (start < list.size()) {
        size_t end = list.find(';', start);
        end = (end == std::string::npos) ? list.size() : end;
        apply(list.substr(start, end - start));
        start = end + 1;
    }
}

} // namespace

const std::vector<Option> &All() {
    return kOptions;
}

std::string CVarName(const Option &option, int port) {
    return std::string("gRemix.Panel.") + option.key + ".P" + std::to_string(port + 1);
}

std::string TagNameCVarName(int tag) {
    return std::string("gRemix.Panel.TagName.") + std::to_string(tag);
}

std::map<int32_t, const char *> ComboMap(const Option &option) {
    std::map<int32_t, const char *> map;
    for (size_t i = 0; i < option.strings.size(); i++) {
        map[option.min + static_cast<int32_t>(i)] = option.strings[i].c_str();
    }
    return map;
}

} // namespace remix_panel

extern "C" void remix_panel_init(void) {
    using namespace remix_panel;

    for (auto &row : sIsOverridden) {
        row.fill(false);
    }
    sIsTagNameOverridden.fill(false);
    const char *tag_names = std::getenv("SSB64_REMIX_TAG_NAMES");
    if (tag_names != nullptr) {
        ForEachListItem(tag_names, ApplyTagNameOverride);
        port_log("SSB64: remix_panel: tag names from SSB64_REMIX_TAG_NAMES applied\n");
    }
    const char *list = std::getenv("SSB64_REMIX_PANEL");
    if (list == nullptr) {
        return;
    }
    ForEachListItem(list, ApplyOverride);
    port_log("SSB64: remix_panel: overrides from SSB64_REMIX_PANEL applied\n");
}

extern "C" const char *remix_panel_tag_name(int tag) {
    using namespace remix_panel;

    if ((tag < 1) || (tag > REMIX_PANEL_TAG_COUNT)) {
        return "";
    }
    std::string &buffer = sTagNameBuffers[tag - 1];
    buffer = sIsTagNameOverridden[tag - 1] ? sTagNameOverrides[tag - 1] : CVarGetString(TagNameCVarName(tag).c_str(), "");
    return buffer.c_str();
}

extern "C" int remix_panel_get(int option, int port) {
    using namespace remix_panel;

    if (!IsValid(option, port)) {
        return 0;
    }
    if (sIsOverridden[option][port]) {
        return sOverrides[option][port];
    }
    const Option &entry = kOptions[option];
    return Clamp(entry, CVarGetInteger(CVarName(entry, port).c_str(), entry.fallback));
}

extern "C" void remix_panel_set(int option, int port, int value) {
    using namespace remix_panel;

    if (!IsValid(option, port)) {
        return;
    }
    const Option &entry = kOptions[option];
    sIsOverridden[option][port] = false;
    CVarSetInteger(CVarName(entry, port).c_str(), Clamp(entry, value));
}

extern "C" void remix_panel_reset(void) {
    using namespace remix_panel;

    for (const Option &option : kOptions) {
        for (int port = 0; port < REMIX_PANEL_PORTS; port++) {
            remix_panel_set(option.id, port, option.fallback);
        }
    }
}
