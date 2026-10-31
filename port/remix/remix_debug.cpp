/**
 * remix_debug.cpp - see remix_debug.h.
 */

#include "remix_debug.h"

#include "../fighter_registry.h"

#include <spdlog/spdlog.h>

#include <cstdlib>

namespace {

constexpr int kDemoSlotCount = 4;

/* Parses the slot-th comma-separated integer of `text`; -1 when missing. */
int parseNthFkind(const char *text, int slot)
{
    const char *cursor = text;
    for (int index = 0; index <= slot; ++index) {
        char *end = nullptr;
        const long value = std::strtol(cursor, &end, 10);
        if (end == cursor) {
            return -1;
        }
        if (index == slot) {
            return static_cast<int>(value);
        }
        if (*end != ',') {
            return -1;
        }
        cursor = end + 1;
    }
    return -1;
}

/* The registered fkind in slot `slot` of env var `name`; -1 when unset or invalid. */
int readSlotFkind(const char *name, int slot)
{
    const char *text = std::getenv(name);
    if (text == nullptr || text[0] == '\0' || slot < 0 || slot >= kDemoSlotCount) {
        return -1;
    }
    const int fkind = parseNthFkind(text, slot);
    if (fkind < 0 || port_fighter_descriptor(fkind) == nullptr) {
        spdlog::warn("remix_debug: {}='{}' slot {} is not a registered fkind; ignored", name, text, slot);
        return -1;
    }
    spdlog::warn("remix_debug: {} slot {} forced to fkind {}", name, slot, fkind);
    return fkind;
}

} /* namespace */

int remix_debug_demo_fkind(int slot, int fallback)
{
    const int fkind = readSlotFkind("SSB64_DEMO_FKINDS", slot);
    return (fkind < 0) ? fallback : fkind;
}

int remix_debug_vs_fkind(int slot)
{
    return readSlotFkind("SSB64_VS_FKINDS", slot);
}

int remix_debug_train_fkind(int slot)
{
    return readSlotFkind("SSB64_TRAIN_FKINDS", slot);
}

int remix_debug_vs_item_rate(void)
{
    const char *text = std::getenv("SSB64_VS_ITEMS");
    if (text == nullptr || text[0] == '\0') {
        return -1;
    }
    return std::atoi(text);
}

int remix_debug_force_item_kind(void)
{
    const char *text = std::getenv("SSB64_REMIX_FORCE_ITEM");
    if (text == nullptr || text[0] == '\0') {
        return -1;
    }
    return std::atoi(text);
}

int remix_debug_force_item_at_p1(void)
{
    const char *text = std::getenv("SSB64_REMIX_FORCE_ITEM_AT_P1");
    return (text != nullptr && text[0] != '\0' && text[0] != '0') ? 1 : 0;
}

const char *remix_debug_env_text(const char *name)
{
    const char *text = std::getenv(name);
    return (text != nullptr && text[0] != '\0') ? text : nullptr;
}

int remix_debug_env_int(const char *name, int fallback)
{
    const char *text = remix_debug_env_text(name);
    return (text != nullptr) ? std::atoi(text) : fallback;
}
