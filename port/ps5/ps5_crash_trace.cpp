/*
 * Crash reporting for the PS5 title, which has no debugger or console: a fatal signal or an
 * uncaught C++ exception appends its cause to the boot trace (/data/BattleShip-Remix/boot.txt).
 * The load address of TraceFatalSignal is traced at install time so a faulting RIP maps back to eboot.elf.
 */
#ifdef __PS5__

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <initializer_list>
#include <signal.h>
#include <sys/ucontext.h>
#include <typeinfo>
#include <cxxabi.h>

extern "C" void ps5_boot_trace(const char* step);

namespace {

constexpr int kMaxTracedFrames = 24;

/**
 * Walks the saved frame-pointer chain (the title is built with -fno-omit-frame-pointer) and traces
 * each return address; frames inside system modules without frame pointers end the walk early.
 * @param frame_pointer the faulting thread's rbp
 */
void TraceFrameChain(uintptr_t frame_pointer) {
    for (int depth = 0; depth < kMaxTracedFrames && frame_pointer != 0 && (frame_pointer & 7u) == 0; depth++) {
        const uintptr_t* frame = reinterpret_cast<const uintptr_t*>(frame_pointer);
        char line[64];
        snprintf(line, sizeof(line), "  frame %d ret=%#lx", depth, static_cast<unsigned long>(frame[1]));
        ps5_boot_trace(line);
        if (frame[0] <= frame_pointer) {
            return;
        }
        frame_pointer = frame[0];
    }
}

void TraceFatalSignal(int signal_number, siginfo_t* info, void* context) {
    const ucontext_t* user_context = static_cast<const ucontext_t*>(context);
    char line[160];
    snprintf(line, sizeof(line), "FATAL signal %d addr=%p rip=%#lx handler=%p", signal_number,
             info != nullptr ? info->si_addr : nullptr,
             user_context != nullptr ? static_cast<unsigned long>(user_context->uc_mcontext.mc_rip) : 0UL,
             reinterpret_cast<void*>(&TraceFatalSignal));
    ps5_boot_trace(line);
    if (user_context != nullptr) {
        TraceFrameChain(static_cast<uintptr_t>(user_context->uc_mcontext.mc_rbp));
    }
    _Exit(128 + signal_number);
}

void TraceUncaughtException() {
    char line[256];
    std::type_info* type = abi::__cxa_current_exception_type();
    try {
        std::exception_ptr pending = std::current_exception();
        if (pending) {
            std::rethrow_exception(pending);
        }
        snprintf(line, sizeof(line), "TERMINATE without an active exception");
    } catch (const std::exception& error) {
        snprintf(line, sizeof(line), "TERMINATE %s: %s", type != nullptr ? type->name() : "?", error.what());
    } catch (...) {
        snprintf(line, sizeof(line), "TERMINATE non-std exception %s", type != nullptr ? type->name() : "?");
    }
    ps5_boot_trace(line);
    _Exit(134);
}

} // namespace

/**
 * Routes fatal signals and std::terminate to the boot trace. Call once, first thing in main.
 */
extern "C" void ps5_install_crash_trace(void) {
    struct sigaction action = {};
    action.sa_sigaction = TraceFatalSignal;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    for (int signal_number : { SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT }) {
        sigaction(signal_number, &action, nullptr);
    }
    std::set_terminate(TraceUncaughtException);

    char line[64];
    snprintf(line, sizeof(line), "crash trace installed handler=%p", reinterpret_cast<void*>(&TraceFatalSignal));
    ps5_boot_trace(line);
}

#endif
