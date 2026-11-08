#include "port_log.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

static FILE *sLogFile = NULL;
static int sTraceEnabled = 0;
static struct timespec sLogStart;

static double port_log_timespec_ms(const struct timespec *ts)
{
	return (double)ts->tv_sec * 1000.0 + (double)ts->tv_nsec / 1000000.0;
}

/* devkitA64's newlib has no timespec_get; clock_gettime is available on every POSIX-like target
 * while MSVC's UCRT only offers timespec_get. */
static void port_log_now(struct timespec *ts)
{
#ifdef _WIN32
	timespec_get(ts, TIME_UTC);
#else
	clock_gettime(CLOCK_MONOTONIC, ts);
#endif
}

void port_log_init(const char *path)
{
	if (sLogFile != NULL) return;
	port_log_now(&sLogStart);
	sLogFile = fopen(path, "w");
#if defined(__PS5__) || defined(__SWITCH__)
	/* Each write to the console's storage costs milliseconds (tens of them on the PS5 /data; the
	 * Switch SD card goes through the HOS fs service with newlib's 1 KB stdio buffer): unbuffered
	 * logging slowed the PS5 20-50x and stuttered the Switch. Crashes still reach the boot trace
	 * (port/ps5/ps5_crash_trace.cpp) and the HOS crash reporter. */
	if (sLogFile != NULL) {
		setvbuf(sLogFile, NULL, _IOFBF, 64 * 1024);
	}
#endif
	const char *trace = getenv("SSB64_TRACE");
	sTraceEnabled = (trace != NULL && trace[0] != '\0' && trace[0] != '0');
}

int port_trace_enabled(void)
{
	return sTraceEnabled;
}

void port_log_close(void)
{
	if (sLogFile != NULL) {
		fclose(sLogFile);
		sLogFile = NULL;
	}
}

int port_log_get_fd(void)
{
	if (sLogFile == NULL) return -1;
	return fileno(sLogFile);
}

void port_log(const char *fmt, ...)
{
	if (sLogFile == NULL) return;
	va_list ap;
	va_start(ap, fmt);
	vfprintf(sLogFile, fmt, ap);
	va_end(ap);
	#if defined(BATTLESHIP_UWP)
	/* Xbox Device Portal may terminate a failed UWP process before stdio's
	 * normal-exit flush runs. Keep startup diagnostics durable so failures
	 * can be diagnosed from LocalState without attaching a debugger. */
	fflush(sLogFile);
	#endif
	/* fflush on every call costs seconds per frame on a slow drive when
	 * figatree watchdogs fire 28x per frame during a stuck APPEAR. Rely on
	 * stdio's buffer + OS-on-exit flush for normal logging; crash dumps
	 * have their own flush path. */
}

void port_log_flush(void)
{
	if (sLogFile != NULL) fflush(sLogFile);
}

double port_log_uptime_ms(void)
{
	struct timespec now;
	port_log_now(&now);
	return port_log_timespec_ms(&now) - port_log_timespec_ms(&sLogStart);
}

void port_trace(const char *fmt, ...)
{
	if (!sTraceEnabled || sLogFile == NULL) return;
	va_list ap;
	va_start(ap, fmt);
	vfprintf(sLogFile, fmt, ap);
	va_end(ap);
}

void port_exit_process(int code)
{
	if (sLogFile != NULL) fflush(sLogFile);
	fflush(NULL);
#ifdef _WIN32
	/* Not _exit()/ExitProcess(): those kill the other threads and then run
	 * every DLL's DLL_PROCESS_DETACH on this thread, which fail-fasts when a
	 * killed thread held one of their locks (seen as STATUS_FAIL_FAST_EXCEPTION
	 * at VS scene teardown). TerminateProcess skips all of that. */
	TerminateProcess(GetCurrentProcess(), (UINT)code);
	/* Only reached if TerminateProcess itself failed; never return into the game. */
	_exit(code);
#else
	_exit(code);
#endif
}
