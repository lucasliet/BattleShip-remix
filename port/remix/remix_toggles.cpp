/**
 * remix_toggles.cpp - see remix_toggles.h.
 */

#include "remix_toggles.h"

#include <libultraship/bridge/consolevariablebridge.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <unordered_map>

extern "C" {
#include "port_log.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
}

namespace remix_toggles {
namespace {

constexpr uint32_t kNodeType = 0x00;
constexpr uint32_t kNodeMin = 0x08;
constexpr uint32_t kNodeMax = 0x0C;
constexpr uint32_t kNodeAction = 0x10;
constexpr uint32_t kNodeStrings = 0x14;
constexpr uint32_t kNodeNext = 0x1C;
constexpr uint32_t kNodeTitle = 0x28;
constexpr uint32_t kTypeTitle = 0;
constexpr uint32_t kTypeBool = 1;
constexpr size_t kTitleMax = 96;
constexpr size_t kNodeLimit = 4096;
constexpr int kExpectedToggles = 531;

const uint32_t kHeads[GroupCount] = {
    REMIX_SYM_Toggles__head_remix_settings,    REMIX_SYM_Toggles__head_gameplay_settings,
    REMIX_SYM_Toggles__head_music_settings,    REMIX_SYM_Toggles__head_stage_settings,
    REMIX_SYM_Toggles__head_pokemon_settings,
};
const uint32_t kProfileDefaults[REMIX_TOGGLE_PROFILE_COUNT] = {
    REMIX_SYM_Toggles__profile_defaults_CE,
    REMIX_SYM_Toggles__profile_defaults_TE,
    REMIX_SYM_Toggles__profile_defaults_NE,
    REMIX_SYM_Toggles__profile_defaults_JP,
};
const char *const kGroupNames[GroupCount] = { "Remix Settings", "Gameplay Settings", "Music Settings", "Stage Settings",
                                              "Pokemon Settings" };

std::vector<Toggle> sToggles;
std::unordered_map<uint32_t, size_t> sByAddress;
std::unordered_map<size_t, int> sOverrides;

uint32_t ReadWord(uint32_t addr) {
    const uint32_t *word = static_cast<const uint32_t *>(remix_ram_image_resolve(addr));
    return (word != nullptr) ? *word : 0;
}

uint8_t ReadByte(uint32_t addr) {
    return static_cast<uint8_t>(ReadWord(addr & ~3u) >> (8 * (3 - (addr & 3))));
}

std::string ReadString(uint32_t addr) {
    std::string text;
    for (size_t i = 0; (addr != 0) && (i < kTitleMax); i++) {
        const uint8_t c = ReadByte(addr + static_cast<uint32_t>(i));
        if (c == 0) {
            break;
        }
        text.push_back(static_cast<char>(c));
    }
    return text;
}

std::string Lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return std::tolower(c); });
    return text;
}

/* Non-title nodes of one group, in chain order. */
void CollectGroup(Group group, std::vector<Toggle> &out) {
    uint32_t node = kHeads[group];
    for (size_t guard = 0; (node != 0) && (guard < kNodeLimit); guard++, node = ReadWord(node + kNodeNext)) {
        const uint32_t type = ReadWord(node + kNodeType);
        if (type == kTypeTitle) {
            continue;
        }
        Toggle toggle{};
        toggle.entryAddr = node;
        toggle.group = group;
        toggle.title = ReadString(node + kNodeTitle);
        toggle.min = static_cast<int>(ReadWord(node + kNodeMin));
        toggle.max = static_cast<int>(ReadWord(node + kNodeMax));
        toggle.isBool = (type == kTypeBool);
        toggle.hasAction = (ReadWord(node + kNodeAction) != 0);
        const uint32_t table = ReadWord(node + kNodeStrings);
        for (int v = toggle.min; (table != 0) && (v <= toggle.max) && (v < 64); v++) {
            toggle.strings.push_back(ReadString(ReadWord(table + static_cast<uint32_t>(v - toggle.min) * 4)));
        }
        out.push_back(std::move(toggle));
    }
}

int Clamp(const Toggle &toggle, int value) {
    return std::max(toggle.min, std::min(toggle.max, value));
}

int ValueOf(size_t index) {
    const auto override = sOverrides.find(index);
    if (override != sOverrides.end()) {
        return override->second;
    }
    const Toggle &toggle = sToggles[index];
    return Clamp(toggle, CVarGetInteger(toggle.cvar.c_str(), toggle.defaults[0]));
}

void ApplyEnvironment() {
    if (const char *profile = std::getenv("SSB64_REMIX_PROFILE")) {
        const int number = std::atoi(profile);
        if ((number >= 0) && (number < REMIX_TOGGLE_PROFILE_COUNT)) {
            for (size_t i = 0; i < sToggles.size(); i++) {
                sOverrides[i] = Clamp(sToggles[i], sToggles[i].defaults[number]);
            }
        }
    }
    const char *list = std::getenv("SSB64_REMIX_TOGGLES");
    if (list == nullptr) {
        return;
    }
    std::string rest(list);
    size_t start = 0;
    while (start < rest.size()) {
        size_t end = rest.find(';', start);
        end = (end == std::string::npos) ? rest.size() : end;
        const std::string item = rest.substr(start, end - start);
        const size_t equals = item.find('=');
        start = end + 1;
        if (equals == std::string::npos) {
            continue;
        }
        const std::string title = Lower(item.substr(0, equals));
        const int value = std::atoi(item.c_str() + equals + 1);
        bool found = false;
        for (size_t i = 0; i < sToggles.size(); i++) {
            if (Lower(sToggles[i].title) == title) {
                sOverrides[i] = Clamp(sToggles[i], value);
                found = true;
            }
        }
        if (!found) {
            spdlog::warn("remix_toggles: SSB64_REMIX_TOGGLES names no toggle '{}'", title);
        }
    }
}

} // namespace

const char *GroupName(Group group) {
    return kGroupNames[group];
}

const std::vector<Toggle> &All() {
    return sToggles;
}

std::map<int32_t, const char *> ComboMap(const Toggle &toggle) {
    std::map<int32_t, const char *> map;
    for (size_t i = 0; i < toggle.strings.size(); i++) {
        map[toggle.min + static_cast<int32_t>(i)] = toggle.strings[i].c_str();
    }
    return map;
}

} // namespace remix_toggles

extern "C" int remix_toggles_init(void) {
    using namespace remix_toggles;

    sToggles.clear();
    sByAddress.clear();
    sOverrides.clear();
    for (int group = 0; group < GroupCount; group++) {
        CollectGroup(static_cast<Group>(group), sToggles);
    }
    std::sort(sToggles.begin(), sToggles.end(),
              [](const Toggle &a, const Toggle &b) { return a.entryAddr < b.entryAddr; });
    if (sToggles.size() != kExpectedToggles) {
        spdlog::error("remix_toggles: found {} toggles, expected {}; the image does not hold the Remix 2.0.1 tables",
                      sToggles.size(), kExpectedToggles);
        sToggles.clear();
        return 0;
    }
    for (size_t i = 0; i < sToggles.size(); i++) {
        Toggle &toggle = sToggles[i];
        toggle.index = static_cast<int>(i);
        toggle.cvar = "gRemix.Toggle." + std::to_string(i);
        for (int profile = 0; profile < REMIX_TOGGLE_PROFILE_COUNT; profile++) {
            toggle.defaults[profile] = static_cast<int>(ReadWord(kProfileDefaults[profile] + static_cast<uint32_t>(i) * 4));
        }
        sByAddress[toggle.entryAddr] = i;
    }
    ApplyEnvironment();
    port_log("SSB64: remix_toggles: %zu toggles, %zu overridden for this run\n", sToggles.size(), sOverrides.size());
    return static_cast<int>(sToggles.size());
}

extern "C" int remix_toggle_get(uint32_t entry_addr) {
    using namespace remix_toggles;

    const auto found = sByAddress.find(entry_addr);
    return (found != sByAddress.end()) ? ValueOf(found->second) : 0;
}

extern "C" void remix_toggle_set(uint32_t entry_addr, int value) {
    using namespace remix_toggles;

    const auto found = sByAddress.find(entry_addr);
    if (found == sByAddress.end()) {
        return;
    }
    const Toggle &toggle = sToggles[found->second];
    sOverrides.erase(found->second);
    CVarSetInteger(toggle.cvar.c_str(), Clamp(toggle, value));
}

extern "C" void remix_toggles_load_profile(int profile) {
    using namespace remix_toggles;

    if ((profile < 0) || (profile >= REMIX_TOGGLE_PROFILE_COUNT)) {
        return;
    }
    for (size_t i = 0; i < sToggles.size(); i++) {
        sOverrides.erase(i);
        CVarSetInteger(sToggles[i].cvar.c_str(), Clamp(sToggles[i], sToggles[i].defaults[profile]));
    }
}

static_assert(remix_toggles::Gameplay == REMIX_TOGGLE_GROUP_GAMEPLAY, "gameplay group number");
static_assert(remix_toggles::Pokemon == REMIX_TOGGLE_GROUP_POKEMON, "pokemon group number");

extern "C" int remix_toggle_group_snapshot(int group, int *values, int *maxima, int capacity) {
    using namespace remix_toggles;

    if ((group < 0) || (group >= GroupCount)) {
        return 0;
    }
    int count = 0;
    for (size_t i = 0; (i < sToggles.size()) && (count < capacity); i++) {
        if (sToggles[i].group != static_cast<Group>(group)) {
            continue;
        }
        values[count] = ValueOf(i);
        maxima[count] = sToggles[i].max;
        count++;
    }
    return count;
}
