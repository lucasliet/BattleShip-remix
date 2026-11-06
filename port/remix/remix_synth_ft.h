/**
 * remix_synth_ft.h - C API that builds a synth fighter's FTData.
 *
 * Implemented in decomp/src/ft/ftport.c, because FTData and FTMotionDesc need
 * the decomp headers (ssb_types.h), which the C++ port side cannot include.
 * The C++ loader passes plain values only.
 */

#ifndef PORT_REMIX_REMIX_SYNTH_FT_H
#define PORT_REMIX_REMIX_SYNTH_FT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PORT_SYNTH_FILE_COUNT 9

struct FTData;

/* One motion row as stored in the remix/fighters blob (native endianness). */
typedef struct PortSynthMotion {
    uint32_t anim_file_id;
    uint32_t command_offset;
    uint32_t flags;
} PortSynthMotion;

/* Remix-supplied fields for one synth FTData. Everything else is copied from
 * the parent's FTData. */
typedef struct PortSynthFtSpec {
    uint32_t file_ids[PORT_SYNTH_FILE_COUNT]; /* main, mainmotion, submotion, model, shieldpose, special1..4 */
    uint32_t file_main_size;
    uint32_t o_attributes;
    uint32_t file_anim_size;
    const PortSynthMotion *mainmotion;
    uint32_t mainmotion_count;
    const PortSynthMotion *submotion;
    uint32_t submotion_count;
} PortSynthFtSpec;

/* Builds an FTData that clones parent_fkind's FTData, overridden by spec.
 * The result owns its motion arrays and p_file_* / count slots. Release it
 * with port_fighter_synth_ft_destroy. Returns NULL if parent_fkind has no
 * registered FTData or on allocation failure. */
struct FTData *port_fighter_synth_ft_create(int parent_fkind, const PortSynthFtSpec *spec);

/* Grows a vanilla fighter's main motion array in place to `count` rows: the
 * first `vanilla_count` rows stay as they are, the rest come from `rows` (Remix
 * adds motions to vanilla fighters, e.g. Kirby's copy abilities). Returns 0 when
 * nothing changed. */
int port_fighter_extend_mainmotion(int fkind, uint32_t vanilla_count, const PortSynthMotion *rows, uint32_t count);

/* Frees an FTData returned by port_fighter_synth_ft_create. NULL is ignored. */
void port_fighter_synth_ft_destroy(struct FTData *ft);

/* Remix moved the attributes block inside the main files of vanilla fkinds 13..25.
 * Points the registered FTData of `fkind` at the Remix offset; returns 0 if it has none. */
int port_fighter_set_attributes_offset(int fkind, uint32_t o_attributes);

/* Character.asm copy_menu_actions: copies the menu motion rows [begin, begin + count) of base_fkind into the menu motion array
 * of target_fkind, first growing that array to begin + count rows when it is shorter. The row count of the grown array
 * replaces the vanilla one so the figatree heap is sized for the copied animations (the asm keeps the vanilla count pointer;
 * the N64 heap size does not depend on it). Returns 0 when nothing changed. */
int port_fighter_copy_menu_motions(int target_fkind, int base_fkind, uint32_t begin, uint32_t count);

/* Character.asm `origin 0x97A4C + 0x68`: target_fkind uses the menu motion array of base_fkind (Giant DK shares DK's). The
 * row count is shared too, for the reason given above. Returns 0 when nothing changed. */
int port_fighter_share_menu_motions(int target_fkind, int base_fkind);

#ifdef __cplusplus
}
#endif

#endif /* PORT_REMIX_REMIX_SYNTH_FT_H */
