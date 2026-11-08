#pragma once

/**
 * port_log.h — Unified crash-safe logging for the SSB64 PC port.
 *
 * All port logging goes through port_log(). Output is written to a single
 * file (ssb64.log) with immediate fflush after every write, so nothing is
 * lost on crash. Call port_log_init() once at the very start of main(),
 * before any LUS initialization (which redirects stderr).
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Open the log file. Call once, before anything else. */
void port_log_init(const char *path);

/* Close the log file. Call at shutdown. */
void port_log_close(void);

/* Return the raw file descriptor of the log file, or -1 if not open.
 * Used by async-signal-safe crash handlers that cannot call fprintf. */
int port_log_get_fd(void);

/* Write a formatted message to the log file. */
#ifdef __GNUC__
__attribute__((format(printf, 1, 2)))
#endif
void port_log(const char *fmt, ...);

/* Push the buffered log to disk. The game loop calls it once a second, so a process the console kills
 * (closing from the HOME menu on the Switch) loses at most the last second of the log. */
void port_log_flush(void);

/* Milliseconds since port_log_init: boot milestones and scene loads log it so a slow start on a console
 * (the Switch SD card, the PS5 /data) can be attributed from the log alone. */
double port_log_uptime_ms(void);

/* Per-frame diagnostics (fighter status changes, GObj ejects, collision probes): written only when
 * the SSB64_TRACE environment variable is set at start-up. Left on, they filled the Switch log at
 * about 14 MB per half hour and each buffer flush stalled the frame on the SD card. */
#ifdef __GNUC__
__attribute__((format(printf, 1, 2)))
#endif
void port_trace(const char *fmt, ...);

/* TRUE when port_trace writes, for callers that build an expensive message first. */
int port_trace_enabled(void);

/* Flush the log (and all stdio streams) and terminate the process with
 * `code` immediately: no destructors, no atexit handlers, no DLL detach
 * callbacks. For batch/rig modes that report a verdict through the exit
 * code from inside a game coroutine, where normal teardown is not safe
 * (render/audio threads are mid-frame). */
void port_exit_process(int code);

#ifdef __cplusplus
}
#endif
