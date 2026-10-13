/**
 * port_input_script.cpp — see port_input_script.h.
 */

#include "port_input_script.h"

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "port_log.h"

namespace {

struct ScriptLine {
    int from;
    int to;
    int port;
    uint16_t button;
    int8_t stickX;
    int8_t stickY;
};

const std::map<std::string, uint16_t> kButtonBits = {
    { "A", 0x8000 },  { "B", 0x4000 },  { "Z", 0x2000 },  { "START", 0x1000 }, { "DU", 0x0800 },
    { "DD", 0x0400 }, { "DL", 0x0200 }, { "DR", 0x0100 }, { "L", 0x0020 },     { "R", 0x0010 },
    { "CU", 0x0008 }, { "CD", 0x0004 }, { "CL", 0x0002 }, { "CR", 0x0001 },
};

bool sLoaded = false;
std::vector<ScriptLine> sLines;
int sFrame = 0;

bool parseButtons(const std::string &spec, uint16_t &out)
{
    out = 0;
    if (spec == "-") {
        return true;
    }
    std::stringstream names(spec);
    std::string name;
    while (std::getline(names, name, '+')) {
        const auto bit = kButtonBits.find(name);
        if (bit == kButtonBits.end()) {
            port_log("SSB64: input script: unknown button '%s' (expected A,B,Z,START,L,R,D*,C*)\n", name.c_str());
            return false;
        }
        out |= bit->second;
    }
    return true;
}

bool parseLine(const std::string &text, ScriptLine &line)
{
    std::istringstream fields(text);
    std::string buttons;
    int stickX = 0;
    int stickY = 0;
    if (!(fields >> line.from >> line.to >> line.port >> buttons)) {
        return false;
    }
    fields >> stickX >> stickY;
    line.stickX = static_cast<int8_t>(stickX);
    line.stickY = static_cast<int8_t>(stickY);
    return parseButtons(buttons, line.button);
}

void loadScriptOnce()
{
    if (sLoaded) {
        return;
    }
    sLoaded = true;

    const char *path = std::getenv("SSB64_INPUT_SCRIPT");
    if (path == nullptr || path[0] == '\0') {
        return;
    }
    std::ifstream file(path);
    if (!file) {
        port_log("SSB64: input script: cannot open '%s'\n", path);
        return;
    }
    std::string text;
    int lineNo = 0;
    while (std::getline(file, text)) {
        lineNo++;
        text = text.substr(0, text.find('#'));
        if (text.find_first_not_of(" \t\r") == std::string::npos) {
            continue;
        }
        ScriptLine line{};
        if (!parseLine(text, line)) {
            port_log("SSB64: input script: %s:%d malformed, expected '<from> <to> <port> <buttons> [sx sy]'\n", path,
                     lineNo);
            continue;
        }
        sLines.push_back(line);
    }
    port_log("SSB64: input script: %zu lines loaded from '%s'\n", sLines.size(), path);
}

} /* namespace */

extern "C" void port_input_script_tick(void)
{
    sFrame++;
}

extern "C" int port_input_script_override(int port, unsigned short *button, signed char *stick_x, signed char *stick_y)
{
    loadScriptOnce();
    for (const ScriptLine &line : sLines) {
        if (line.port != port || sFrame < line.from || sFrame > line.to) {
            continue;
        }
        *button = line.button;
        *stick_x = line.stickX;
        *stick_y = line.stickY;
        return 1;
    }
    return 0;
}
