/**
 * remix_modes.cpp - see remix_modes.h.
 */

#include "remix_modes.h"

#include <libultraship/bridge/consolevariablebridge.h>

extern "C" int remix_modes_smashketball_type(void) {
    return CVarGetInteger(REMIX_MODES_CVAR_SMASHKETBALL_TYPE, 0) != 0 ? 1 : 0;
}
