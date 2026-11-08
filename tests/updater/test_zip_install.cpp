#include "UpdateZipInstall.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include <sys/stat.h>
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

struct ModedEntry {
    std::string name;
    std::string content;
    unsigned unixMode;
};

void WriteZipWithModes(const std::string& path, const std::vector<ModedEntry>& entries) {
    int error = 0;
    zip_t* archive = zip_open(path.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    for (const ModedEntry& entry : entries) {
        zip_source_t* source = zip_source_buffer(archive, entry.content.data(), entry.content.size(), 0);
        const zip_int64_t index = zip_file_add(archive, entry.name.c_str(), source, ZIP_FL_OVERWRITE);
        if (entry.unixMode == 0) continue;
        zip_file_set_external_attributes(archive, static_cast<zip_uint64_t>(index), 0, ZIP_OPSYS_UNIX,
                                         static_cast<zip_uint32_t>(entry.unixMode) << 16);
    }
    zip_close(archive);
}

void WriteZip(const std::string& path, const ZipEntries& entries) {
    std::vector<ModedEntry> moded;
    for (const auto& [name, content] : entries) {
        moded.push_back(ModedEntry{ name, content, 0 });
    }
    WriteZipWithModes(path, moded);
}

bool HasPermission(const std::string& path, std::filesystem::perms permission) {
    return (std::filesystem::status(path).permissions() & permission) != std::filesystem::perms::none;
}

int CountEntriesBelow(const std::string& root) {
    if (!std::filesystem::exists(root)) return 0;
    int count = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
        if (!std::filesystem::is_directory(entry.symlink_status())) ++count;
    }
    return count;
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

void testExtractStripsNestedPrefixIntoNewFolder() {
    // Given
    const std::string dir = MakeScratchDirectory("extract_prefix");
    const std::string staging = dir + "/staging";
    WriteZip(dir + "/u.zip", { { "switch/BattleShip-Remix/BattleShip-Remix.nro", "nro" },
                               { "switch/BattleShip-Remix/assets/yamls/a.yml", "yaml" },
                               { "switch/other/skipped.txt", "skipped" },
                               { "README.txt", "outside the prefix" } });
    ZipExtractOptions options;
    options.rootPrefix = "switch/BattleShip-Remix/";

    // When
    const ZipInstallStatus status = ExtractZipTo(dir + "/u.zip", options, staging);

    // Then
    CHECK(status == ZipInstallStatus::Installed);
    CHECK(ReadFile(staging + "/BattleShip-Remix.nro") == "nro");
    CHECK(ReadFile(staging + "/assets/yamls/a.yml") == "yaml");
    CHECK(!std::filesystem::exists(staging + "/README.txt"));
    CHECK(!std::filesystem::exists(staging + "/skipped.txt"));
    CHECK(CountEntriesBelow(staging) == 2);
    for (const auto& entry : std::filesystem::recursive_directory_iterator(staging)) {
        CHECK(entry.path().extension() != ".part");
    }
}

void testExtractFromArchiveKeepsStoredModes() {
    // Given
    const std::string dir = MakeScratchDirectory("extract_modes");
    const std::string staging = dir + "/staging";
    WriteZipWithModes(dir + "/u.zip", { { "App/bin/run", "binary", 0755u }, { "App/data.txt", "data", 0644u } });
    ZipExtractOptions options;
    options.rootPrefix = "App/";
    options.fileMode = ZipFileMode::FromArchive;

    // When
    const ZipInstallStatus status = ExtractZipTo(dir + "/u.zip", options, staging);

    // Then
    CHECK(status == ZipInstallStatus::Installed);
    CHECK(HasPermission(staging + "/bin/run", std::filesystem::perms::owner_exec));
    CHECK(!HasPermission(staging + "/data.txt", std::filesystem::perms::owner_exec));
}

void testExtractIgnoreModeWritesReadableFiles() {
    // Given
    const std::string dir = MakeScratchDirectory("extract_ignore");
    const std::string staging = dir + "/staging";
    WriteZipWithModes(dir + "/u.zip", { { "App/bin/run", "binary", 0755u } });
    ZipExtractOptions options;
    options.rootPrefix = "App/";
    options.fileMode = ZipFileMode::Ignore;

    // When
    const ZipInstallStatus status = ExtractZipTo(dir + "/u.zip", options, staging);

    // Then
    CHECK(status == ZipInstallStatus::Installed);
    CHECK(std::filesystem::exists(staging + "/bin/run"));
    CHECK(ReadFile(staging + "/bin/run") == "binary");
}

void testExtractExecutableModeMarksFilesExecutable() {
    // Given
    const std::string dir = MakeScratchDirectory("extract_executable");
    const std::string staging = dir + "/staging";
    WriteZipWithModes(dir + "/u.zip", { { "App/data.txt", "data", 0644u } });
    ZipExtractOptions options;
    options.rootPrefix = "App/";
    options.fileMode = ZipFileMode::Executable;

    // When
    const ZipInstallStatus status = ExtractZipTo(dir + "/u.zip", options, staging);

    // Then
    CHECK(status == ZipInstallStatus::Installed);
    CHECK(HasPermission(staging + "/data.txt", std::filesystem::perms::owner_exec));
    CHECK(HasPermission(staging + "/data.txt", std::filesystem::perms::others_exec));
}

void testExtractCreatesAllowedSymlink() {
    // Given
    const std::string dir = MakeScratchDirectory("extract_symlink");
    const std::string staging = dir + "/staging";
    WriteZipWithModes(dir + "/u.zip", { { "App/libSDL3.0.dylib", "lib", 0644u },
                                        { "App/libSDL3.dylib", "libSDL3.0.dylib", (S_IFLNK | 0777u) } });
    ZipExtractOptions options;
    options.rootPrefix = "App/";
    options.allowSymlinks = true;

    // When
    const ZipInstallStatus status = ExtractZipTo(dir + "/u.zip", options, staging);

    // Then
    CHECK(status == ZipInstallStatus::Installed);
    CHECK(std::filesystem::is_symlink(staging + "/libSDL3.dylib"));
    CHECK(std::filesystem::read_symlink(staging + "/libSDL3.dylib") == "libSDL3.0.dylib");
    CHECK(ReadFile(staging + "/libSDL3.dylib") == "lib");
}

void testExtractRejectsSymlinkWhenNotAllowedAndCleansUp() {
    // Given
    const std::string dir = MakeScratchDirectory("extract_symlink_denied");
    const std::string staging = dir + "/staging";
    WriteZipWithModes(dir + "/u.zip", { { "App/sub/lib.dylib", "lib", 0644u },
                                        { "App/sub/link.dylib", "lib.dylib", (S_IFLNK | 0777u) } });
    ZipExtractOptions options;
    options.rootPrefix = "App/";
    options.allowSymlinks = false;

    // When
    const ZipInstallStatus status = ExtractZipTo(dir + "/u.zip", options, staging);

    // Then
    CHECK(status == ZipInstallStatus::UnsafeEntry);
    CHECK(CountEntriesBelow(staging) == 0);
}

void testExtractRejectsSymlinkEscapingTheTree() {
    // Given
    const std::string dir = MakeScratchDirectory("extract_symlink_escape");
    const std::string staging = dir + "/staging";
    WriteZipWithModes(dir + "/u.zip", { { "App/link", "../escape", (S_IFLNK | 0777u) } });
    ZipExtractOptions options;
    options.rootPrefix = "App/";
    options.allowSymlinks = true;

    // When
    const ZipInstallStatus status = ExtractZipTo(dir + "/u.zip", options, staging);

    // Then
    CHECK(status == ZipInstallStatus::UnsafeEntry);
    CHECK(CountEntriesBelow(staging) == 0);
}

void testExtractReportsBadZipsAndEmptyPrefixes() {
    // Given
    const std::string dir = MakeScratchDirectory("extract_bad");
    WriteFile(dir + "/not.zip", "not a zip");
    WriteZip(dir + "/other.zip", { { "OTHER/file.bin", "x" } });
    ZipExtractOptions options;
    options.rootPrefix = "App/";

    // When
    const ZipInstallStatus notZip = ExtractZipTo(dir + "/not.zip", options, dir + "/staging");
    const ZipInstallStatus otherRoot = ExtractZipTo(dir + "/other.zip", options, dir + "/staging");

    // Then
    CHECK(notZip == ZipInstallStatus::OpenFailed);
    CHECK(otherRoot == ZipInstallStatus::NoEntries);
    CHECK(!std::filesystem::exists(dir + "/staging"));
}

} // namespace

int RunZipInstallTests() {
    testInstallsFilesUnderTheRootAndKeepsOthers();
    testRejectsUnsafePathsAndKeepsTheInstall();
    testReportsBadZipsAndEmptyRoots();
    testExtractStripsNestedPrefixIntoNewFolder();
    testExtractFromArchiveKeepsStoredModes();
    testExtractIgnoreModeWritesReadableFiles();
    testExtractExecutableModeMarksFilesExecutable();
    testExtractCreatesAllowedSymlink();
    testExtractRejectsSymlinkWhenNotAllowedAndCleansUp();
    testExtractRejectsSymlinkEscapingTheTree();
    testExtractReportsBadZipsAndEmptyPrefixes();
    return s_failures;
}
