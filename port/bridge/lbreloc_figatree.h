#pragma once

/**
 * lbreloc_figatree.h — marks extra reloc files as fighter figatree streams.
 *
 * Figatree files need the u16 halfswap on load. Vanilla ones are detected by
 * their "reloc_animations/FT" path; Smash Remix's new animations are unnamed
 * ("reloc_misc/file_NNNN") and must be registered by file id before loading.
 */

#ifdef __cplusplus
extern "C" {
#endif

void portRelocMarkFighterFigatreeFile(unsigned int file_id);

#ifdef __cplusplus
}
#endif
