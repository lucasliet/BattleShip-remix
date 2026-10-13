#pragma once

/**
 * port_input_script.h — scripted controller input for headless debug runs.
 *
 * SSB64_INPUT_SCRIPT=<path> names a text file whose lines override one
 * controller port for a range of main-loop frames:
 *
 *     <from> <to> <port> <buttons> [<stick_x> <stick_y>]
 *
 * `<buttons>` is `-` or names joined by `+` (A, B, Z, START, L, R, DU, DD,
 * DL, DR, CU, CD, CL, CR). Frames are inclusive and use the same numbering
 * as SSB64_MAX_FRAMES. `#` starts a comment. Unset means no override.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Advances the script clock; call once per main-loop frame. */
void port_input_script_tick(void);

/**
 * Replaces the pad state of `port` when a script line covers the current
 * frame. Returns 1 when the pad was overridden, 0 otherwise.
 */
int port_input_script_override(int port, unsigned short *button, signed char *stick_x, signed char *stick_y);

#ifdef __cplusplus
}
#endif
