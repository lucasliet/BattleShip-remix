/**
 * reloc_size_cache.h — on-disk copy of lbRelocGetFileSize results.
 *
 * The stage select sizes every stage file on its first entry, which reads a few hundred archive headers
 * (422 ms on the Switch SD card). The sizes only change with the archives, so they are stored in the app
 * directory under a key made of the asset recipe hash and every mounted archive's path and size; a key
 * mismatch discards the file.
 */
#ifndef PORT_BRIDGE_RELOC_SIZE_CACHE_H
#define PORT_BRIDGE_RELOC_SIZE_CACHE_H

#include <cstddef>
#include <cstdint>
#include <unordered_map>

using RelocFileSizeMap = std::unordered_map<uint32_t, size_t>;

/**
 * Reads the stored sizes into sizes when the stored key matches the mounted archives.
 * @return true when sizes were loaded.
 */
bool portRelocSizeCacheLoad(RelocFileSizeMap &sizes);

/** Writes sizes under the current archive key, replacing the previous file. */
void portRelocSizeCacheSave(const RelocFileSizeMap &sizes);

#endif
