/*
 * System notifications (the console's pop-up) for the PS5 title. They work before the game
 * window exists, so first-launch extraction can tell the user what is happening.
 */
#ifdef __PS5__

#include <string.h>

typedef struct notify_request {
    char reserved[45];
    char message[3075];
} notify_request_t;

int sceKernelSendNotificationRequest(int device, notify_request_t *request, size_t size, int blocking);

/**
 * Shows a system notification.
 * @param message the text, truncated to the request's buffer
 */
void ps5_notify(const char *message) {
    notify_request_t request;
    memset(&request, 0, sizeof request);
    strncpy(request.message, message, sizeof request.message - 1);
    sceKernelSendNotificationRequest(0, &request, sizeof request, 0);
}

#endif
