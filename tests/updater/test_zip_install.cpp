#include "UpdateZipInstall.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include <zip.h>

using namespace ssb64::enhancements;

namespace {

int s_failures = 0;

#define CHECK(condition)                                                                     \
    do {                                                                                     \
        if (!(condition)) {                                                                  \
            std::cerr << __FILE__ << ":" << __LINE__ << " CHECK failed: " #condition "\n"; \
            ++s_failures;                                                                    \
        }                                                                                    \
    } while (0)

using ZipEntries = std::vector<std::pair<std::string, std::string>>;

std::string MakeScratchDirectory(const std::string& name) {
    std::filesystem::path dir = std::filesystem::temp_directory_path() / ("zip_install_tests_" + name);
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir / "title");
    return dir.string();
}

void WriteFile(const std::string& path, const std::string& content) {
    std::ofstream(path, std::ios::binary) << content;
}

std::string ReadFile(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

void WriteZip(const std::string& path, const ZipEntries& entries) {
    int error = 0;
    zip_t* archive = zip_open(path.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    for (const auto& [name, content] : entries) {
        zip_source_t* source = zip_source_buffer(archive, content.data(), content.size(), 0);
        zip_file_add(archive, name.c_str(), source, ZIP_FL_OVERWRITE);
    }
    zip_close(archive);
}

void testInstallsFilesUnderTheRootAndKeepsOthers() {
    // Given
    const std::string dir = MakeScratchDirectory("install");
    const std::string title = dir + "/title";
    WriteFile(title + "/eboot.bin", "old eboot");
    WriteFile(title + "/baserom.remix.z64", "rom");
    WriteZip(dir + "/u.zip", { { "PPSA99264/eboot.bin", "new eboot" },
                               { "PPSA99264/assets/yamls/a.yml", "yaml" },
                               { "README.txt", "outside the root" } });

    // When
    const ZipInstallStatus status = InstallTitleZip(dir + "/u.zip", "PPSA99264/", title);

    // Then
    CHECK(status == ZipInstallStatus::Installed);
    CHECK(ReadFile(title + "/eboot.bin") == "new eboot");
    CHECK(ReadFile(title + "/assets/yamls/a.yml") == "yaml");
    CHECK(ReadFile(title + "/baserom.remix.z64") == "rom");
    CHECK(!std::filesystem::exists(title + "/README.txt"));
    CHECK(!std::filesystem::exists(title + "/eboot.bin.part"));
    const auto mode = std::filesystem::status(title + "/eboot.bin").permissions();
    CHECK((mode & std::filesystem::perms::owner_exec) != std::filesystem::perms::none);
    CHECK((mode & std::filesystem::perms::others_exec) != std::filesystem::perms::none);
}

void testRejectsUnsafePathsAndKeepsTheInstall() {
    // Given
    const std::string dir = MakeScratchDirectory("unsafe");
    const std::string title = dir + "/title";
    WriteFile(title + "/eboot.bin", "old eboot");
    WriteZip(dir + "/u.zip", { { "PPSA99264/eboot.bin", "new eboot" }, { "PPSA99264/../escape.txt", "x" } });

    // When
    const ZipInstallStatus status = InstallTitleZip(dir + "/u.zip", "PPSA99264/", title);

    // Then
    CHECK(status == ZipInstallStatus::UnsafeEntry);
    CHECK(ReadFile(title + "/eboot.bin") == "old eboot");
    CHECK(!std::filesystem::exists(dir + "/escape.txt"));
}

void testReportsBadZipsAndEmptyRoots() {
    // Given
    const std::string dir = MakeScratchDirectory("bad");
    WriteFile(dir + "/not.zip", "not a zip");
    WriteZip(dir + "/other.zip", { { "OTHER/eboot.bin", "x" } });

    // When
    const ZipInstallStatus notZip = InstallTitleZip(dir + "/not.zip", "PPSA99264/", dir + "/title");
    const ZipInstallStatus otherRoot = InstallTitleZip(dir + "/other.zip", "PPSA99264/", dir + "/title");

    // Then
    CHECK(notZip == ZipInstallStatus::OpenFailed);
    CHECK(otherRoot == ZipInstallStatus::NoEntries);
}

} // namespace

int RunZipInstallTests() {
    testInstallsFilesUnderTheRootAndKeepsOthers();
    testRejectsUnsafePathsAndKeepsTheInstall();
    testReportsBadZipsAndEmptyRoots();
    return s_failures;
}
