/*
 * Boot breadcrumbs for the PS5 title: appends one line per step to boot.txt in the title folder
 * (/app0) with plain POSIX calls, so a crash before ssb64.log is open still leaves the last step reached.
 */
#ifdef __PS5__

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

static const char *const kBootTracePath = "/app0/boot.txt";

/**
 * Appends a step name to the boot trace.
 * @param step a short label for the step reached
 */
void ps5_boot_trace(const char *step) {
    int fd = open(kBootTracePath, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd < 0) {
        return;
    }
    write(fd, step, strlen(step));
    write(fd, "\n", 1);
    close(fd);
}

__attribute__((constructor(101))) static void ps5_boot_trace_first_constructor(void) {
    unlink(kBootTracePath);
    ps5_boot_trace("static constructors started");
}

#endif
