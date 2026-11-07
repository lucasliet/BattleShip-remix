/*
 * Startup micro-benchmarks for the PS5 title: the game ran 20-50x slower than real time on the
 * console, so this times the CPU, memory, the allocator, sleeps and file reads and writes each result
 * to the boot trace (/data/BattleShip-Remix/boot.txt).
 */
#ifdef __PS5__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void ps5_boot_trace(const char *step);

static const char *const kProbeFile = "/data/BattleShip-Remix/BattleShip-Remix.o2r";

/**
 * Reads the monotonic clock.
 * @return microseconds since an arbitrary origin
 */
static uint64_t probe_now_us(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t)now.tv_sec * 1000000u + (uint64_t)now.tv_nsec / 1000u;
}

/**
 * Writes one probe result to the boot trace.
 * @param name the probe name
 * @param started_us the probe start from probe_now_us
 */
static void probe_report(const char *name, uint64_t started_us) {
    char line[96];
    snprintf(line, sizeof(line), "perf %s: %llu us", name, (unsigned long long)(probe_now_us() - started_us));
    ps5_boot_trace(line);
}

static void probe_cpu(void) {
    volatile uint32_t accumulator = 1;
    uint64_t started = probe_now_us();
    for (uint32_t index = 0; index < 100000000u; index++) {
        accumulator = accumulator * 1664525u + index;
    }
    probe_report("cpu 100M mul-add", started);
}

static void probe_memory(void) {
    const size_t size = 64u * 1024u * 1024u;
    uint64_t started = probe_now_us();
    unsigned char *block = malloc(size);
    if (block == NULL) {
        ps5_boot_trace("perf memset: malloc failed");
        return;
    }
    memset(block, 1, size);
    probe_report("malloc+memset 64 MiB first touch", started);
    started = probe_now_us();
    memset(block, 2, size);
    probe_report("memset 64 MiB again", started);
    free(block);
}

static void probe_allocator(void) {
    void *blocks[256];
    uint64_t started = probe_now_us();
    for (int round = 0; round < 400; round++) {
        for (int index = 0; index < 256; index++) {
            blocks[index] = malloc((size_t)(16 + index * 24));
        }
        for (int index = 0; index < 256; index++) {
            free(blocks[index]);
        }
    }
    probe_report("malloc/free 102400 pairs", started);
}

static void probe_sleep(void) {
    const struct timespec one_ms = { 0, 1000000 };
    uint64_t started = probe_now_us();
    for (int index = 0; index < 10; index++) {
        nanosleep(&one_ms, NULL);
    }
    probe_report("10x nanosleep 1 ms", started);
}

static void probe_file_read(void) {
    FILE *file = fopen(kProbeFile, "rb");
    if (file == NULL) {
        ps5_boot_trace("perf fread: open failed");
        return;
    }
    static unsigned char chunk[4096];
    size_t total = 0;
    uint64_t started = probe_now_us();
    while (total < 16u * 1024u * 1024u) {
        size_t read = fread(chunk, 1, sizeof(chunk), file);
        if (read == 0) {
            break;
        }
        total += read;
    }
    probe_report("fread 16 MiB in 4 KiB chunks", started);
    started = probe_now_us();
    for (int index = 0; index < 2000; index++) {
        fseek(file, (long)((index * 7919u) % (8u * 1024u * 1024u)), SEEK_SET);
        fread(chunk, 1, 64, file);
    }
    probe_report("2000 fseek+fread 64 B", started);
    fclose(file);
}

/**
 * Runs every probe once. Call from main before the engine starts.
 */
void ps5_run_perf_probe(void) {
    probe_cpu();
    probe_memory();
    probe_allocator();
    probe_sleep();
    probe_file_read();
}

#endif
