/*
 * Path calls a PS5 title is refused (EPERM): lstat, getcwd and realpath, which std::filesystem's
 * directory iteration, status, canonical and relative are built on. A title's folders hold no
 * symbolic links, so lstat answers as stat and a path is resolved by its text; the title folder
 * answers as the working directory. Same answers as griches/PS5_Ghostship ps5/src/shims.c. The
 * link wraps the three names (PS5_EXTRA_WRAPS in CMakeLists.txt).
 */
#ifdef __PS5__

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define PS5_TITLE_DIR "/app0"
#define PS5_PATH_SIZE 1024

int __wrap_lstat(const char *path, struct stat *out) {
    return stat(path, out);
}

char *__wrap_getcwd(char *buffer, size_t size) {
    if (buffer == NULL) {
        size = size == 0 ? PS5_PATH_SIZE : size;
        buffer = malloc(size);
        if (buffer == NULL) {
            return NULL;
        }
    }
    if (size < sizeof PS5_TITLE_DIR) {
        errno = ERANGE;
        return NULL;
    }
    memcpy(buffer, PS5_TITLE_DIR, sizeof PS5_TITLE_DIR);
    return buffer;
}

/**
 * Normalises an absolute path by its text: drops ".", applies "..", collapses slashes.
 * @param full the absolute path
 * @param out receives the normalised path (PS5_PATH_SIZE bytes)
 * @return the normalised length
 */
static size_t normalise_path(const char *full, char *out) {
    size_t length = 0;
    for (const char *part = full; *part != '\0';) {
        while (*part == '/') {
            part++;
        }
        const size_t size = strcspn(part, "/");
        if (size == 2 && part[0] == '.' && part[1] == '.') {
            while (length > 0 && out[--length] != '/') {
            }
        } else if (size > 0 && !(size == 1 && part[0] == '.')) {
            out[length++] = '/';
            memcpy(out + length, part, size);
            length += size;
        }
        part += size;
    }
    if (length == 0) {
        out[length++] = '/';
    }
    out[length] = '\0';
    return length;
}

char *__wrap_realpath(const char *path, char *resolved) {
    char full[PS5_PATH_SIZE];
    char out[PS5_PATH_SIZE];
    struct stat status;
    if (path == NULL || *path == '\0') {
        errno = path == NULL ? EINVAL : ENOENT;
        return NULL;
    }
    if ((size_t)snprintf(full, sizeof full, "%s%s", path[0] == '/' ? "" : PS5_TITLE_DIR "/", path) >= sizeof full) {
        errno = ENAMETOOLONG;
        return NULL;
    }
    const size_t length = normalise_path(full, out);
    if (stat(out, &status) != 0) {
        errno = ENOENT;
        return NULL;
    }
    if (resolved == NULL) {
        resolved = malloc(PS5_PATH_SIZE);
        if (resolved == NULL) {
            return NULL;
        }
    }
    memcpy(resolved, out, length + 1);
    return resolved;
}

#endif
