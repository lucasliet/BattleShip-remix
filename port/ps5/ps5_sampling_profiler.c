/*
 * Sampling profiler for the PS5 title, which has no debugger: a helper thread interrupts the main
 * thread (where the game coroutines run) every few milliseconds with SIGUSR2 and the handler records
 * the interrupted RIP. The console's ucontext layout does not match the SDK header, so the RIP slot
 * is found at startup by interrupting a known spin loop. The hottest addresses go to the boot trace.
 */
#ifdef __PS5__

#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void ps5_boot_trace(const char *step);

enum {
    kContextWordsScanned = 256,
    kSampleCapacity = 6000,
    kReportedAddresses = 40,
    kSampleIntervalNs = 2000000,
    kStartDelaySeconds = 15,
    kSpinWindowBytes = 512,
    kCallerDepth = 3,
    /* FreeBSD mcontext keeps rbp 11 words and rsp 3 words from rip; the console shifts all by 6. */
    kRbpWordsBeforeRip = 11,
    kRspWordsAfterRip = 3,
    kStackWindowBytes = 4 * 1024 * 1024,
};

static pthread_t sProfiledThread;
static volatile int sRipWordIndex = -1;
static volatile int sCalibrating;
static volatile int sSpinRunning;
static uintptr_t sSamples[kSampleCapacity];
static uintptr_t sCallers[kSampleCapacity][kCallerDepth];
static volatile int sSampleCount;

__attribute__((noinline)) static void calibration_spin(void) {
    while (sSpinRunning) {
        __asm__ volatile("" ::: "memory");
    }
}

static void record_calibration(const uintptr_t *words) {
    const uintptr_t begin = (uintptr_t)&calibration_spin;
    for (int index = 0; index < kContextWordsScanned; index++) {
        if (words[index] >= begin && words[index] < begin + kSpinWindowBytes) {
            sRipWordIndex = index;
            return;
        }
    }
}

static int is_stack_address(uintptr_t address, uintptr_t anchor) {
    return address > anchor && address < anchor + kStackWindowBytes && (address & 7u) == 0;
}

/**
 * Records the return address at the interrupted rsp (callers of leaf system stubs) and the next
 * frame-pointer return addresses.
 * @param words the console ucontext as words
 * @param callers receives kCallerDepth return addresses, 0 when unreadable
 */
static void record_callers(const uintptr_t *words, uintptr_t *callers) {
    volatile uintptr_t anchor_marker = 0;
    const uintptr_t anchor = (uintptr_t)&anchor_marker;
    memset(callers, 0, sizeof(uintptr_t) * kCallerDepth);
    uintptr_t rsp = words[sRipWordIndex + kRspWordsAfterRip];
    if (is_stack_address(rsp, anchor)) {
        callers[0] = *(const uintptr_t *)rsp;
    }
    uintptr_t rbp = words[sRipWordIndex - kRbpWordsBeforeRip];
    for (int depth = 1; depth < kCallerDepth && is_stack_address(rbp, anchor); depth++) {
        const uintptr_t *frame = (const uintptr_t *)rbp;
        callers[depth] = frame[1];
        rbp = frame[0];
    }
}

static void on_sample_signal(int signal_number, siginfo_t *info, void *context) {
    (void)signal_number;
    (void)info;
    const uintptr_t *words = (const uintptr_t *)context;
    if (sCalibrating) {
        record_calibration(words);
        return;
    }
    int index = sSampleCount;
    if (sRipWordIndex < 0 || index >= kSampleCapacity) {
        return;
    }
    sSamples[index] = words[sRipWordIndex];
    record_callers(words, sCallers[index]);
    sSampleCount = index + 1;
}

static void sleep_ns(long nanoseconds) {
    const struct timespec delay = { nanoseconds / 1000000000L, nanoseconds % 1000000000L };
    nanosleep(&delay, NULL);
}

static void *calibration_signaler(void *unused) {
    (void)unused;
    for (int attempt = 0; attempt < 50 && sRipWordIndex < 0; attempt++) {
        sleep_ns(kSampleIntervalNs);
        pthread_kill(sProfiledThread, SIGUSR2);
    }
    sSpinRunning = 0;
    return NULL;
}

static int compare_chains(const void *left, const void *right) {
    return memcmp(left, right, sizeof(uintptr_t) * kCallerDepth);
}

static int compare_addresses(const void *left, const void *right) {
    uintptr_t a = *(const uintptr_t *)left;
    uintptr_t b = *(const uintptr_t *)right;
    return a < b ? -1 : a > b;
}

/**
 * Collapses the sorted samples into (address, count) runs and traces the most frequent ones.
 * @param count the number of samples
 */
static void report_hottest(int count) {
    qsort(sSamples, (size_t)count, sizeof(sSamples[0]), compare_addresses);
    char line[80];
    snprintf(line, sizeof(line), "prof samples=%d rip_word=%d", count, sRipWordIndex);
    ps5_boot_trace(line);
    for (int reported = 0; reported < kReportedAddresses; reported++) {
        int best_start = -1;
        int best_length = 0;
        for (int start = 0; start < count;) {
            int end = start;
            while (end < count && sSamples[end] == sSamples[start]) {
                end++;
            }
            if (sSamples[start] != 0 && end - start > best_length) {
                best_start = start;
                best_length = end - start;
            }
            start = end;
        }
        if (best_start < 0) {
            return;
        }
        snprintf(line, sizeof(line), "prof hits=%d code=%#lx", best_length, (unsigned long)sSamples[best_start]);
        ps5_boot_trace(line);
        for (int index = best_start; index < best_start + best_length; index++) {
            sSamples[index] = 0;
        }
    }
}

/**
 * Traces the most frequent caller chains among the samples whose RIP is outside the title.
 * @param count the number of samples
 */
static void report_system_callers(int count) {
    static uintptr_t chains[kSampleCapacity][kCallerDepth];
    int chain_count = 0;
    for (int index = 0; index < count; index++) {
        if (sSamples[index] >= 0x800000000ul) {
            memcpy(chains[chain_count++], sCallers[index], sizeof(chains[0]));
        }
    }
    qsort(chains, (size_t)chain_count, sizeof(chains[0]), compare_chains);
    char line[128];
    for (int reported = 0; reported < 20; reported++) {
        int best_start = -1;
        int best_length = 0;
        for (int start = 0; start < chain_count;) {
            int end = start;
            while (end < chain_count && memcmp(chains[end], chains[start], sizeof(chains[0])) == 0) {
                end++;
            }
            if (chains[start][0] != 1 && end - start > best_length) {
                best_start = start;
                best_length = end - start;
            }
            start = end;
        }
        if (best_start < 0) {
            return;
        }
        snprintf(line, sizeof(line), "prof sys hits=%d code=%#lx code=%#lx code=%#lx", best_length,
                 (unsigned long)chains[best_start][0], (unsigned long)chains[best_start][1],
                 (unsigned long)chains[best_start][2]);
        ps5_boot_trace(line);
        for (int index = best_start; index < best_start + best_length; index++) {
            chains[index][0] = 1;
        }
    }
}

static void *sampling_signaler(void *unused) {
    (void)unused;
    sleep_ns((long)kStartDelaySeconds * 1000000000L);
    ps5_boot_trace("prof sampling started");
    while (sSampleCount < kSampleCapacity) {
        sleep_ns(kSampleIntervalNs);
        pthread_kill(sProfiledThread, SIGUSR2);
    }
    sleep_ns(kSampleIntervalNs);
    report_system_callers(sSampleCount);
    report_hottest(sSampleCount);
    return NULL;
}

/**
 * Finds the RIP slot of the console's ucontext, then starts sampling the calling thread after
 * kStartDelaySeconds. Call once from the main thread before the engine starts.
 */
void ps5_start_sampling_profiler(void) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = on_sample_signal;
    action.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&action.sa_mask);
    sigaction(SIGUSR2, &action, NULL);
    sProfiledThread = pthread_self();

    pthread_t helper;
    sCalibrating = 1;
    sSpinRunning = 1;
    if (pthread_create(&helper, NULL, calibration_signaler, NULL) != 0) {
        ps5_boot_trace("prof calibration thread failed");
        return;
    }
    calibration_spin();
    pthread_join(helper, NULL);
    sCalibrating = 0;

    char line[96];
    snprintf(line, sizeof(line), "prof rip_word=%d spin=%p", sRipWordIndex, (void *)&calibration_spin);
    ps5_boot_trace(line);
    if (sRipWordIndex < 0) {
        return;
    }
    if (pthread_create(&helper, NULL, sampling_signaler, NULL) == 0) {
        pthread_detach(helper);
    }
}

#endif
