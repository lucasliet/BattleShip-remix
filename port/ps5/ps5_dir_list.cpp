/*
 * Folder listing on the PS5 without opendir, which the title is refused under /app0 (EPERM): the
 * hi-res pack scan of mods/ goes through here.
 */
#ifdef __PS5__

#include "ps5_dir_list.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace ssb64 {
namespace {

constexpr int kDirentBufferSize = 64 * 1024;

std::string DescribeFailure(const char* call, const std::string& path) {
    return std::string(call) + " " + path + ": " + std::strerror(errno);
}

bool IsDotEntry(const char* name) {
    return std::strcmp(name, ".") == 0 || std::strcmp(name, "..") == 0;
}

/**
 * Tells a folder from a file when getdents gives no type.
 * @return the dirent type of the path, DT_UNKNOWN when stat fails
 */
unsigned char TypeOf(const std::string& path, unsigned char direntType) {
    if (direntType != DT_UNKNOWN) {
        return direntType;
    }
    struct stat status;
    if (stat(path.c_str(), &status) != 0) {
        return DT_UNKNOWN;
    }
    return S_ISDIR(status.st_mode) ? DT_DIR : (S_ISREG(status.st_mode) ? DT_REG : DT_UNKNOWN);
}

bool ListInto(const std::string& directory, std::vector<std::string>* files, std::string* error) {
    const int fd = open(directory.c_str(), O_RDONLY | O_DIRECTORY);
    if (fd < 0) {
        *error = DescribeFailure("open", directory);
        return false;
    }
    std::vector<std::string> subdirectories;
    std::vector<char> buffer(kDirentBufferSize);
    bool ok = true;
    for (;;) {
        const int bytes = getdents(fd, buffer.data(), kDirentBufferSize);
        if (bytes < 0) {
            *error = DescribeFailure("getdents", directory);
            ok = false;
            break;
        }
        if (bytes == 0) {
            break;
        }
        for (int offset = 0; offset < bytes;) {
            const auto* entry = reinterpret_cast<const struct dirent*>(buffer.data() + offset);
            if (entry->d_reclen == 0) {
                break;
            }
            offset += entry->d_reclen;
            if (entry->d_fileno == 0 || IsDotEntry(entry->d_name)) {
                continue;
            }
            const std::string path = directory + "/" + entry->d_name;
            const unsigned char type = TypeOf(path, entry->d_type);
            if (type == DT_DIR) {
                subdirectories.push_back(path);
            } else if (type == DT_REG) {
                files->push_back(path);
            }
        }
    }
    close(fd);
    for (const std::string& subdirectory : subdirectories) {
        if (!ok) {
            break;
        }
        ok = ListInto(subdirectory, files, error);
    }
    return ok;
}

} // namespace

bool Ps5ListFilesRecursive(const std::string& directory, std::vector<std::string>* files, std::string* error) {
    files->clear();
    if (!ListInto(directory, files, error)) {
        return false;
    }
    std::sort(files->begin(), files->end());
    return true;
}

} // namespace ssb64

#endif
