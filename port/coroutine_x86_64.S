// coroutine_x86_64.S — context-switch primitive for the x86_64 System V
// coroutine backend (PS5 native title, FreeBSD-derived ABI).
//
// The SysV AMD64 calling convention defines this callee-saved set:
//
//   GP : rbx, rbp, r12..r15, rsp
//   FPU: the MXCSR control bits and the x87 control word
//
// Everything else is caller-saved, so port_coroutine_swap only saves and
// restores that set. The return address sits on the stack: the swap stores
// rsp as left by the `call` (pointing at the return address), restores the
// other coroutine's rsp and `ret`s into whatever address that stack holds.
//
// Context layout (64 bytes). Offsets are mirrored by static_asserts on the
// C `CoroCtx` struct in coroutine_android.cpp:
//
//      0   rbx   |  32   r14
//      8   rbp   |  40   r15
//     16   r12   |  48   rsp
//     24   r13   |  56   mxcsr (4 bytes) + x87 cw (2 bytes) + pad

#if defined(__x86_64__)

    .text

// void port_coroutine_swap(CoroCtx *from, CoroCtx *to);
//   rdi = from, rsi = to.
    .globl  port_coroutine_swap
    .type   port_coroutine_swap, @function
port_coroutine_swap:
    movq    %rbx,  0(%rdi)
    movq    %rbp,  8(%rdi)
    movq    %r12, 16(%rdi)
    movq    %r13, 24(%rdi)
    movq    %r14, 32(%rdi)
    movq    %r15, 40(%rdi)
    movq    %rsp, 48(%rdi)
    stmxcsr 56(%rdi)
    fnstcw  60(%rdi)

    movq     0(%rsi), %rbx
    movq     8(%rsi), %rbp
    movq    16(%rsi), %r12
    movq    24(%rsi), %r13
    movq    32(%rsi), %r14
    movq    40(%rsi), %r15
    movq    48(%rsi), %rsp
    ldmxcsr 56(%rsi)
    fldcw   60(%rsi)
    ret
    .size   port_coroutine_swap, .-port_coroutine_swap

// void port_coroutine_trampoline_x86_64(void);
//
// First entry point of a fresh coroutine: its initial stack holds this
// address where the swap's `ret` pops it, so rsp is 16-byte aligned here
// (as after a jump, not a call). r12 carries the PortCoroutine*.
    .globl  port_coroutine_trampoline_x86_64
    .type   port_coroutine_trampoline_x86_64, @function
port_coroutine_trampoline_x86_64:
    xorl    %ebp, %ebp
    movq    %r12, %rdi
    call    port_coroutine_trampoline_c
    ud2
    .size   port_coroutine_trampoline_x86_64, .-port_coroutine_trampoline_x86_64

#endif /* __x86_64__ */

#if defined(__ELF__)
    .section .note.GNU-stack,"",%progbits
#endif
