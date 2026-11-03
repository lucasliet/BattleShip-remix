/*
 * input-script.c - scripted controller input plugin for mupen64plus (reference emulator, M11 I8).
 *
 * Reads REF_EMU_INPUT, a file in the same format as the port's SSB64_INPUT_SCRIPT ("<from> <to> <port> <buttons|-> [stick_x
 * stick_y]", buttons joined by '+': A B Z START L R DU DD DL DR CU CD CL CR), so one script can drive the port and the
 * emulator. Frame numbers count the GetKeys calls of port 0, one per controller poll. Only this repository's script
 * plugin is ours; the emulator it plugs into is GPL and lives in .toolchain/ref-emu.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define M64P_PLUGIN_PROTOTYPES 1
#include "m64p_types.h"
#include "m64p_plugin.h"

#define MAX_LINES 4096
#define MAX_PORTS 4

typedef struct
{
    int from;
    int to;
    int port;
    unsigned short wire_buttons;
    signed char stick_x;
    signed char stick_y;
} ScriptLine;

static ScriptLine sLines[MAX_LINES];
static int sLineCount;
static int sFrame;

static const struct { const char *name; unsigned short bit; } kButtons[] = {
    { "A", 0x8000 }, { "B", 0x4000 }, { "Z", 0x2000 }, { "START", 0x1000 }, { "DU", 0x0800 }, { "DD", 0x0400 },
    { "DL", 0x0200 }, { "DR", 0x0100 }, { "L", 0x0020 }, { "R", 0x0010 }, { "CU", 0x0008 }, { "CD", 0x0004 },
    { "CL", 0x0002 }, { "CR", 0x0001 },
};

static int parseButtons(char *spec, unsigned short *wire)
{
    char *name;
    size_t i;

    *wire = 0;
    if (strcmp(spec, "-") == 0)
    {
        return 1;
    }
    for (name = strtok(spec, "+"); name != NULL; name = strtok(NULL, "+"))
    {
        int found = 0;

        for (i = 0; i < sizeof(kButtons) / sizeof(kButtons[0]); i++)
        {
            if (strcmp(name, kButtons[i].name) == 0)
            {
                *wire |= kButtons[i].bit;
                found = 1;
            }
        }
        if (!found)
        {
            fprintf(stderr, "input-script: unknown button '%s'\n", name);
            return 0;
        }
    }
    return 1;
}

static void loadScript(void)
{
    const char *path = getenv("REF_EMU_INPUT");
    char text[256];
    FILE *file;

    if (path == NULL)
    {
        return;
    }
    file = fopen(path, "r");
    if (file == NULL)
    {
        fprintf(stderr, "input-script: cannot open REF_EMU_INPUT '%s'\n", path);
        return;
    }
    while ((sLineCount < MAX_LINES) && (fgets(text, sizeof(text), file) != NULL))
    {
        ScriptLine *line = &sLines[sLineCount];
        char buttons[64];
        int stick_x = 0;
        int stick_y = 0;

        if ((text[0] == '#') || (sscanf(text, "%d %d %d %63s %d %d", &line->from, &line->to, &line->port, buttons, &stick_x, &stick_y) < 4))
        {
            continue;
        }
        line->stick_x = (signed char)stick_x;
        line->stick_y = (signed char)stick_y;
        if (parseButtons(buttons, &line->wire_buttons))
        {
            sLineCount++;
        }
    }
    fclose(file);
}

/* The wire order (A in the high byte) differs from the mupen64plus BUTTONS bit order (A in bit 7). */
static unsigned int toMupenValue(unsigned short wire, signed char stick_x, signed char stick_y)
{
    unsigned int low = (wire >> 8) & 0xFF;
    unsigned int high = (wire & 0xFF) << 8;

    return low | high | ((unsigned int)(unsigned char)stick_x << 16) | ((unsigned int)(unsigned char)stick_y << 24);
}

EXPORT m64p_error CALL PluginStartup(m64p_dynlib_handle core, void *context, void (*debug)(void *, int, const char *))
{
    (void)core; (void)context; (void)debug;
    loadScript();
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginShutdown(void)
{
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginGetVersion(m64p_plugin_type *type, int *version, int *api, const char **name, int *caps)
{
    if (type != NULL) *type = M64PLUGIN_INPUT;
    if (version != NULL) *version = 0x010000;
    if (api != NULL) *api = 0x020100;
    if (name != NULL) *name = "BattleShip scripted input";
    if (caps != NULL) *caps = 0;
    return M64ERR_SUCCESS;
}

EXPORT void CALL InitiateControllers(CONTROL_INFO info)
{
    int port;

    for (port = 0; port < MAX_PORTS; port++)
    {
        info.Controls[port].Present = 1;
        info.Controls[port].RawData = 0;
        info.Controls[port].Plugin = PLUGIN_NONE;
    }
}

EXPORT void CALL GetKeys(int control, BUTTONS *keys)
{
    unsigned short wire = 0;
    signed char stick_x = 0;
    signed char stick_y = 0;
    int i;

    for (i = 0; i < sLineCount; i++)
    {
        if ((sLines[i].port == control) && (sFrame >= sLines[i].from) && (sFrame <= sLines[i].to))
        {
            wire |= sLines[i].wire_buttons;
            stick_x = sLines[i].stick_x ? sLines[i].stick_x : stick_x;
            stick_y = sLines[i].stick_y ? sLines[i].stick_y : stick_y;
        }
    }
    keys->Value = toMupenValue(wire, stick_x, stick_y);
    if (control == 0)
    {
        sFrame++;
    }
}

EXPORT void CALL ControllerCommand(int control, unsigned char *command) { (void)control; (void)command; }
EXPORT void CALL ReadController(int control, unsigned char *command) { (void)control; (void)command; }
EXPORT int CALL RomOpen(void) { sFrame = 0; return 1; }
EXPORT void CALL RomClosed(void) {}
EXPORT void CALL SDL_KeyDown(int keymod, int keysym) { (void)keymod; (void)keysym; }
EXPORT void CALL SDL_KeyUp(int keymod, int keysym) { (void)keymod; (void)keysym; }
EXPORT void CALL RenderCallback(void) {}
