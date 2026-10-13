#pragma once

/**
 * remix_action_bind.h - C++ view of decomp/src/ft/ftremix/ftremixbind.c.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct FighterDescriptor;

/**
 * Fills `descriptor`'s special statuses and special entry handlers from Remix
 * id `remix_id`'s action array; routines without a port fall back to
 * `parent_fkind`'s (the first `parent_action_count` actions mirror the parent's).
 * Returns how many routine slots fell back.
 */
int ftRemixBindSpecials(int remix_id, int parent_fkind, int parent_action_count, struct FighterDescriptor *descriptor);

/**
 * Installs Remix routines on `descriptor`'s per-fighter callbacks (ECB override,
 * shield hitlag skip) for Remix id `remix_id`. Ids without hooks are left as copied
 * from the parent. See decomp/src/ft/ftremix/ftremixdeschooks.c.
 */
void ftRemixBindDescriptorHooks(int remix_id, struct FighterDescriptor *descriptor);

#ifdef __cplusplus
}
#endif
