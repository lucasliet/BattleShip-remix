/**
 * remix_ram_image.h - host view of the Smash Remix RAM image.
 *
 * Remix boots by copying ROM 0x02C00000 to RAM 0x80400000. Data that Remix
 * adds (synth motion scripts and the goto/subroutine targets inside them) is
 * addressed by raw 0x804xxxxx pointers into that image. The image is read
 * from the o2r ("remix/ram_image") and kept with every u32 word converted to
 * native order, the same layout reloc files get after their blanket swap.
 */
#ifndef PORT_REMIX_RAM_IMAGE_H
#define PORT_REMIX_RAM_IMAGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Loads the image from the o2r and registers it as the reloc token fallback.
 * Returns 0 when the resource is missing; resolves then always return NULL.
 */
int remix_ram_image_load(void);

/**
 * Host pointer for a Remix RAM address, or NULL when the address is outside
 * the loaded image or the image is not loaded.
 */
void *remix_ram_image_resolve(uint32_t ram_addr);

#ifdef __cplusplus
}
#endif

#endif
