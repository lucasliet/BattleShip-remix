/*
 * Boot breadcrumbs for the PS5 title: appends one line per step to boot.txt in the title folder
 * (/app0, writable for folder installs), or in /data/BattleShip-Remix otherwise, with plain POSIX calls, so a crash before ssb64.log is open still leaves the last step reached.
 */
#ifdef __PS5__

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static const char *const kTitleBootTracePath = "/app0/boot.txt";
static const char *const kFallbackBootTraceDirectory = "/data/BattleShip-Remix";
static const char *const kFallbackBootTracePath = "/data/BattleShip-Remix/boot.txt";
static const char *sBootTracePath = kFallbackBootTracePath;

/**
 * Appends a step name to the boot trace.
 * @param step a short label for the step reached
 */
void ps5_boot_trace(const char *step) {
    int fd = open(sBootTracePath, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd < 0) {
        return;
    }
    write(fd, step, strlen(step));
    write(fd, "\n", 1);
    close(fd);
}

__attribute__((constructor(101))) static void ps5_boot_trace_first_constructor(void) {
    int fd = open(kTitleBootTracePath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd >= 0) {
        close(fd);
        sBootTracePath = kTitleBootTracePath;
    } else {
        mkdir(kFallbackBootTraceDirectory, 0777);
        unlink(kFallbackBootTracePath);
    }
    ps5_boot_trace("static constructors started");
}

#endif
