/**
 * remix_training.cpp - see remix_training.h.
 */

#include "remix_training.h"

#include <libultraship/bridge/consolevariablebridge.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>

extern "C" {
#include "port_log.h"
}

namespace remix_training {
namespace {

const std::vector<std::string> kOosNames = {
    "Jump", "Roll Left", "Roll Right", "Grab", "Up Smash", "Up Special", "Down Special", "Neutral Air", "Shield Drop",
    "S. Drop DSP", "S. Drop NAir", "None",
};

const std::vector<std::string> kTechNames = { "Random", "Roll Backward", "Roll Forward", "In Place", "Roll Left", "Roll Right", "None" };

const std::vector<std::string> kSpamNames = {
    "Off", "Up Tilt", "Down Tilt", "Forward Tilt", "Up Smash", "Down Smash", "Forward Smash", "Down Special",
    "Neutral Special", "Up Special", "Grab", "Jab", "Short Hop", "Full Hop",
};

const std::vector<std::string> kMusicNames = {
    "Training Mode", "Bonus", "Congo Jungle", "Credits", "Data", "Dream Land", "Duel Zone", "Final Destination", "How to Play",
    "Hyrule Castle", "Meta Crystal", "Mushroom Kingdom", "Peach's Castle", "Planet Zebes", "Saffron City", "Sector Z", "Yoshi's Island",
};

struct CharacterEntry {
    int fkind;
    const char *name;
};

const CharacterEntry kCharacters[] = {
#include "remix_training_characters_data.inc"
};

const int kCharacterCount = static_cast<int>(sizeof(kCharacters) / sizeof(kCharacters[0]));

std::vector<std::string> BuildCharacterNames() {
    std::vector<std::string> names = { "Auto" };
    for (const CharacterEntry &entry : kCharacters) {
        names.push_back(entry.name);
    }
    return names;
}

const std::vector<std::string> kCharacterNames = BuildCharacterNames();
const std::vector<std::string> kTypeNames = { "Auto", "Human", "CPU", "Disabled" };
const std::vector<std::string> kCostumeNames = { "Auto", "1", "2", "3", "4", "5", "6" };
const std::vector<std::string> kSpawnNames = { "Auto", "Port 1", "Port 2", "Port 3", "Port 4" };

const std::vector<Option> kOptions = {
    { REMIX_TRAINING_SHIELD_BREAK, "ShieldBreak", "Shield Break Mode", 0, 2, 0, { "Off", "On", "Infinite" } },
    { REMIX_TRAINING_OOS, "Oos", "OOS Action", 0, 11, 0, kOosNames },
    { REMIX_TRAINING_MUSIC, "Music", "Music", 0, 16, 0, kMusicNames },
    { REMIX_TRAINING_STAGE_BG, "StageBg", "Stage Background", 0, 1, 0, { "Off", "On" } },
    { REMIX_TRAINING_TECH, "Tech", "CPU Teching", 0, 6, 0, kTechNames },
    { REMIX_TRAINING_DI_TYPE, "DiType", "CPU DI Type", 0, 3, 0, { "None", "Random", "Smash", "Slide" } },
    { REMIX_TRAINING_DI_STRENGTH, "DiStrength", "CPU DI Strength", 0, 3, 0, { "Random", "High", "Medium", "Low" } },
    { REMIX_TRAINING_DI_DIRECTION, "DiDirection", "CPU DI Direction", 0, 6, 0,
      { "Random", "Left", "Right", "Up", "Down", "Away", "Toward" } },
    { REMIX_TRAINING_DI_FIRST_HIT, "DiFirstHit", "CPU DI Start on Hit", 1, 99, 2, {} },
    { REMIX_TRAINING_DPAD, "Dpad", "D-pad Controls", 0, 2, 0, { "On", "Reset Only", "Off" } },
    { REMIX_TRAINING_DI_PRACTICE, "DiPractice", "DI Practice Mode", 0, 1, 0, { "Off", "On" } },
    { REMIX_TRAINING_SPAM_PRACTICE, "SpamPractice", "Spam Practice", 0, 13, 0, kSpamNames },
    { REMIX_TRAINING_SPAM_INTERVAL, "SpamInterval", "Spam Interval", 1, 999, 1, {} },
    { REMIX_TRAINING_SPAM_RANDOM, "SpamRandom", "Spam Interval Random", 0, 1, 0, { "Off", "On" } },
    { REMIX_TRAINING_PERCENT_P1, "Percent1", "P1 Percent", 0, 999, 0, {} },
    { REMIX_TRAINING_PERCENT_P2, "Percent2", "P2 Percent", 0, 999, 0, {} },
    { REMIX_TRAINING_PERCENT_P3, "Percent3", "P3 Percent", 0, 999, 0, {} },
    { REMIX_TRAINING_PERCENT_P4, "Percent4", "P4 Percent", 0, 999, 0, {} },
    { REMIX_TRAINING_RESET_SETS_PERCENT_P1, "ResetSetsPercent1", "P1 Reset Sets Percent", 0, 1, 1, { "Off", "On" } },
    { REMIX_TRAINING_RESET_SETS_PERCENT_P2, "ResetSetsPercent2", "P2 Reset Sets Percent", 0, 1, 1, { "Off", "On" } },
    { REMIX_TRAINING_RESET_SETS_PERCENT_P3, "ResetSetsPercent3", "P3 Reset Sets Percent", 0, 1, 1, { "Off", "On" } },
    { REMIX_TRAINING_RESET_SETS_PERCENT_P4, "ResetSetsPercent4", "P4 Reset Sets Percent", 0, 1, 1, { "Off", "On" } },
    { REMIX_TRAINING_TYPE_P1, "Type1", "P1 Type", 0, 3, 0, kTypeNames },
    { REMIX_TRAINING_TYPE_P2, "Type2", "P2 Type", 0, 3, 0, kTypeNames },
    { REMIX_TRAINING_TYPE_P3, "Type3", "P3 Type", 0, 3, 0, kTypeNames },
    { REMIX_TRAINING_TYPE_P4, "Type4", "P4 Type", 0, 3, 0, kTypeNames },
    { REMIX_TRAINING_CHARACTER_P1, "Character1", "P1 Character", 0, kCharacterCount, 0, kCharacterNames },
    { REMIX_TRAINING_CHARACTER_P2, "Character2", "P2 Character", 0, kCharacterCount, 0, kCharacterNames },
    { REMIX_TRAINING_CHARACTER_P3, "Character3", "P3 Character", 0, kCharacterCount, 0, kCharacterNames },
    { REMIX_TRAINING_CHARACTER_P4, "Character4", "P4 Character", 0, kCharacterCount, 0, kCharacterNames },
    { REMIX_TRAINING_COSTUME_P1, "Costume1", "P1 Costume", 0, 6, 0, kCostumeNames },
    { REMIX_TRAINING_COSTUME_P2, "Costume2", "P2 Costume", 0, 6, 0, kCostumeNames },
    { REMIX_TRAINING_COSTUME_P3, "Costume3", "P3 Costume", 0, 6, 0, kCostumeNames },
    { REMIX_TRAINING_COSTUME_P4, "Costume4", "P4 Costume", 0, 6, 0, kCostumeNames },
    { REMIX_TRAINING_SPAWN_P1, "Spawn1", "P1 Spawn", 0, 4, 0, kSpawnNames },
    { REMIX_TRAINING_SPAWN_P2, "Spawn2", "P2 Spawn", 0, 4, 0, kSpawnNames },
    { REMIX_TRAINING_SPAWN_P3, "Spawn3", "P3 Spawn", 0, 4, 0, kSpawnNames },
    { REMIX_TRAINING_SPAWN_P4, "Spawn4", "P4 Spawn", 0, 4, 0, kSpawnNames },
};

std::array<int, REMIX_TRAINING_COUNT> sOverrides;
std::array<bool, REMIX_TRAINING_COUNT> sIsOverridden;

bool IsValid(int option) {
    return (option >= 0) && (option < REMIX_TRAINING_COUNT);
}

int Clamp(const Option &option, int value) {
    return std::max(option.min, std::min(option.max, value));
}

std::string Lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return std::tolower(c); });
    return text;
}

void ApplyOverride(const std::string &item) {
    const size_t equals = item.find('=');
    if (equals == std::string::npos) {
        spdlog::warn("remix_training: SSB64_REMIX_TRAINING item '{}' is not <Key>=<value>", item);
        return;
    }
    const std::string key = Lower(item.substr(0, equals));
    const int value = std::atoi(item.c_str() + equals + 1);
    for (const Option &option : kOptions) {
        if (Lower(option.key) == key) {
            sOverrides[option.id] = Clamp(option, value);
            sIsOverridden[option.id] = true;
            return;
        }
    }
    spdlog::warn("remix_training: SSB64_REMIX_TRAINING item '{}' names no option", item);
}

} // namespace

const std::vector<Option> &All() {
    return kOptions;
}

std::string CVarName(const Option &option) {
    return std::string("gRemix.Training.") + option.key;
}

std::map<int32_t, const char *> ComboMap(const Option &option) {
    std::map<int32_t, const char *> map;
    for (size_t i = 0; i < option.strings.size(); i++) {
        map[option.min + static_cast<int32_t>(i)] = option.strings[i].c_str();
    }
    return map;
}

} // namespace remix_training

extern "C" void remix_training_init(void) {
    using namespace remix_training;

    sIsOverridden.fill(false);
    const char *list = std::getenv("SSB64_REMIX_TRAINING");
    if (list == nullptr) {
        return;
    }
    const std::string rest(list);
    size_t start = 0;
    while (start < rest.size()) {
        size_t end = rest.find(';', start);
        end = (end == std::string::npos) ? rest.size() : end;
        ApplyOverride(rest.substr(start, end - start));
        start = end + 1;
    }
    port_log("SSB64: remix_training: overrides from SSB64_REMIX_TRAINING applied\n");
}

extern "C" int remix_training_character_fkind(int entry_value) {
    using namespace remix_training;

    if ((entry_value < 1) || (entry_value > kCharacterCount)) {
        return -1;
    }
    return kCharacters[entry_value - 1].fkind;
}

extern "C" int remix_training_get(int option) {
    using namespace remix_training;

    if (!IsValid(option)) {
        return 0;
    }
    if (sIsOverridden[option]) {
        return sOverrides[option];
    }
    const Option &entry = kOptions[option];
    return Clamp(entry, CVarGetInteger(CVarName(entry).c_str(), entry.fallback));
}

extern "C" void remix_training_set(int option, int value) {
    using namespace remix_training;

    if (!IsValid(option)) {
        return;
    }
    const Option &entry = kOptions[option];
    sIsOverridden[option] = false;
    CVarSetInteger(CVarName(entry).c_str(), Clamp(entry, value));
}

extern "C" void remix_training_reset(void) {
    using namespace remix_training;

    for (const Option &option : kOptions) {
        remix_training_set(option.id, option.fallback);
    }
}
