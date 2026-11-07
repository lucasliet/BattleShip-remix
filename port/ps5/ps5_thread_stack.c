/*
 * Minimum thread stack for the PS5 title. std::thread cannot choose a stack size and the title's
 * default is small enough that libzip reads on the resource pool overflowed it (SIGILL, then
 * SIGSEGV on the guard page). The link wraps pthread_create (--wrap=pthread_create in
 * scripts/ps5/native-app-stage.sh) so every thread gets at least kMinimumStackSize.
 */
#ifdef __PS5__

#include <pthread.h>
#include <stddef.h>

int __real_pthread_create(pthread_t *thread, const pthread_attr_t *attributes, void *(*start)(void *),
                          void *argument);

static const size_t kMinimumStackSize = 2u * 1024u * 1024u;

/**
 * Creates a thread with the caller's attributes, raising the stack to kMinimumStackSize when the
 * attributes ask for less (or for the default).
 * @return the pthread_create result
 */
int __wrap_pthread_create(pthread_t *thread, const pthread_attr_t *attributes, void *(*start)(void *),
                          void *argument) {
    pthread_attr_t sized;
    if (attributes != NULL) {
        sized = *attributes;
    } else if (pthread_attr_init(&sized) != 0) {
        return __real_pthread_create(thread, attributes, start, argument);
    }
    size_t stack_size = 0;
    if (pthread_attr_getstacksize(&sized, &stack_size) != 0 || stack_size < kMinimumStackSize) {
        pthread_attr_setstacksize(&sized, kMinimumStackSize);
    }
    int result = __real_pthread_create(thread, &sized, start, argument);
    if (attributes == NULL) {
        pthread_attr_destroy(&sized);
    }
    return result;
}

#endif
