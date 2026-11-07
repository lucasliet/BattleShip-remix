/*
 * Heap budget for the PS5 title. ps5-opengl's app_heap.c reserves 128 MiB unless the title defines
 * ps5_opengl_heap_size; the game (a 60 MB archive, decoded resources, audio and the GL driver's CPU
 * side) needs more. A larger size is taken from direct memory, and app_heap.c falls back to smaller
 * sizes when that fails.
 */
#ifdef __PS5__

#include <stddef.h>

const size_t ps5_opengl_heap_size = 1536u * 1024u * 1024u;

#endif
