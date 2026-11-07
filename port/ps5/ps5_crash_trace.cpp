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

constexpr int kStackWordsScanned = 4096;
constexpr int kMaxCodePointers = 48;
constexpr uintptr_t kCodeWindow = 0x1000000;
constexpr uintptr_t kLowestCodeAddress = 0x1000;

/**
 * Traces stack words that point into the title's own code. System modules may run without frame
 * pointers, so this finds the title's callers of a faulting library function; some hits are stale.
 * The linker-provided bounds read as 0 on the console, so the range is a window around this function,
 * wide enough for the whole executable segment (~16 MB).
 * @param stack_pointer the faulting thread's rsp
 */
void TraceStackCodePointers(uintptr_t stack_pointer) {
    const uintptr_t anchor = reinterpret_cast<uintptr_t>(&TraceStackCodePointers);
    const uintptr_t code_begin = anchor > kCodeWindow + kLowestCodeAddress ? anchor - kCodeWindow : kLowestCodeAddress;
    const uintptr_t code_end = anchor + kCodeWindow;
    if (stack_pointer == 0) {
        ps5_boot_trace("  stack scan skipped: no stack pointer");
        return;
    }
    const uintptr_t* stack = reinterpret_cast<const uintptr_t*>(stack_pointer);
    int found = 0;
    for (int index = 0; index < kStackWordsScanned && found < kMaxCodePointers; index++) {
        const uintptr_t word = stack[index];
        if (word < code_begin || word >= code_end) {
            continue;
        }
        char line[64];
        snprintf(line, sizeof(line), "  stack[%d] code=%#lx", index, static_cast<unsigned long>(word));
        ps5_boot_trace(line);
        found++;
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
    // The console's ucontext layout does not match the SDK's FreeBSD header (rsp reads 0), so scan
    // from this handler's own frame: it runs on the faulting thread's stack, below the crash site.
    volatile uintptr_t stack_marker = 0;
    TraceStackCodePointers(reinterpret_cast<uintptr_t>(&stack_marker));
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
