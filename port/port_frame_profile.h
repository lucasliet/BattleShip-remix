/**
 * port_frame_profile.h — opt-in per-frame time counters for hot port hooks.
 *
 * Enabled by SSB64_FRAME_PROFILE=1 (on the Switch through env.txt). The slow-frame log line then also
 * prints how long each category took in that frame and how many calls it made, so a stutter can be
 * attributed on hardware without a profiler. Disabled, each hook costs one predictable branch.
 */
#ifndef PORT_FRAME_PROFILE_H
#define PORT_FRAME_PROFILE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PortProfileCategory {
    PORT_PROFILE_DL_CHECK,
    PORT_PROFILE_VTX_FIXUP,
    PORT_PROFILE_TEX_FIXUP,
    PORT_PROFILE_FILE_LOAD,
    PORT_PROFILE_COUNT
} PortProfileCategory;

extern int gPortFrameProfileEnabled;

/** Reads SSB64_FRAME_PROFILE once; call at startup before the first frame. */
void port_frame_profile_init(void);

/** Monotonic timestamp in nanoseconds, for port_frame_profile_add. */
uint64_t port_frame_profile_now_ns(void);

/** Adds the time since start_ns and one call to the category. */
void port_frame_profile_add(PortProfileCategory category, uint64_t start_ns);

/** Writes "name ms/calls" for every category into buf and clears the counters for the next frame. */
void port_frame_profile_take(char *buf, unsigned long buf_size);

#ifdef __cplusplus
}
#endif

#endif
