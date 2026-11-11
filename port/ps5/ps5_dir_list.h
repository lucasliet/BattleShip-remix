#pragma once

#include <string>
#include <vector>

namespace ssb64 {

/**
 * Lists every regular file below a folder, walking subfolders, with open and getdents. A PS5 title
 * is refused opendir under /app0, which std::filesystem's directory iterators are built on.
 * @param directory the folder to walk
 * @param files receives the full path of each file, sorted
 * @param error receives the call that failed, its path and errno text
 * @return false when a folder could not be read
 */
bool Ps5ListFilesRecursive(const std::string& directory, std::vector<std::string>* files, std::string* error);

} // namespace ssb64
