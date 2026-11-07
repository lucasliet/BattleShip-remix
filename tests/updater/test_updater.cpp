#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "UpdateInstall.h"
#include "UpdateRelease.h"

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

const char* kReleaseBody = R"({
  "tag_name": "v2.0.1-port.3",
  "assets": [
    {"name": "BattleShip-Remix-switch.zip", "size": 5, "browser_download_url": "https://github.com/o/r/releases/download/v/BattleShip-Remix-switch.zip"},
    {"name": "BattleShip-Remix.nro", "size": 1234, "url": "https://api.github.com/repos/o/r/releases/assets/42", "browser_download_url": "https://github.com/o/r/releases/download/v/BattleShip-Remix.nro"}
  ]
})";

std::string MakeScratchDirectory(const std::string& name) {
    std::filesystem::path dir = std::filesystem::temp_directory_path() / ("updater_tests_" + name);
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir.string();
}

void WriteFile(const std::string& path, const std::string& content) {
    std::ofstream(path, std::ios::binary) << content;
}

std::string ReadFile(const std::string& path) {
    std::ifstream stream(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
}

bool Exists(const std::string& path) {
    return std::filesystem::exists(path);
}

std::string FakeNro(const std::string& marker) {
    std::string image(0x10, '\0');
    image += "NRO0";
    image += marker;
    return image;
}

void testTrustedUrlAcceptsOnlyGithubHttps() {
    CHECK(IsTrustedDownloadUrl("https://github.com/o/r/releases/download/v1/x.nro"));
    CHECK(!IsTrustedDownloadUrl("http://github.com/o/r/x.nro"));
    CHECK(!IsTrustedDownloadUrl("https://evil.example/github.com/x.nro"));
    CHECK(!IsTrustedDownloadUrl("https://github.com.evil.example/x.nro"));
    CHECK(!IsTrustedDownloadUrl("https://github.com/o/r/x y.nro"));
    CHECK(!IsTrustedDownloadUrl("https://github.com/o/r/x\".nro"));
    CHECK(!IsTrustedDownloadUrl(""));
}

void testParseFindsNamedAssetWithSize() {
    ReleaseAsset release;
    CHECK(ParseLatestRelease(kReleaseBody, "BattleShip-Remix.nro", &release) == ReleaseParseStatus::Ok);
    CHECK(release.tag == "v2.0.1-port.3");
    CHECK(release.downloadUrl == "https://github.com/o/r/releases/download/v/BattleShip-Remix.nro");
    CHECK(release.sizeBytes == 1234);
    CHECK(release.apiUrl == "https://api.github.com/repos/o/r/releases/assets/42");
}

void testTrustedApiUrlAcceptsOnlyGithubApiHttps() {
    CHECK(IsTrustedApiUrl("https://api.github.com/repos/o/r/releases/assets/42"));
    CHECK(!IsTrustedApiUrl("http://api.github.com/repos/o/r/releases/assets/42"));
    CHECK(!IsTrustedApiUrl("https://api.github.com.evil.example/repos/o/r"));
    CHECK(!IsTrustedApiUrl("https://github.com/o/r/releases/download/v1/x.nro"));
    CHECK(!IsTrustedApiUrl("https://api.github.com/repos/o/r/a b"));
    CHECK(!IsTrustedApiUrl(""));
}

void testParseLeavesApiUrlEmptyWhenUntrusted() {
    const char* body = R"({"tag_name":"v1","assets":[{"name":"a.nro","url":"https://evil.example/repos/a","browser_download_url":"https://github.com/o/r/a.nro"}]})";
    ReleaseAsset release;
    CHECK(ParseLatestRelease(body, "a.nro", &release) == ReleaseParseStatus::Ok);
    CHECK(release.apiUrl.empty());
}

void testParseKeepsTagWhenAssetIsMissing() {
    ReleaseAsset release;
    CHECK(ParseLatestRelease(kReleaseBody, "Other.nro", &release) == ReleaseParseStatus::AssetMissing);
    CHECK(release.tag == "v2.0.1-port.3");
    CHECK(release.downloadUrl.empty());
}

void testParseRejectsUntrustedUrl() {
    const char* body = R"({"tag_name":"v1","assets":[{"name":"a.nro","browser_download_url":"https://evil.example/a.nro"}]})";
    ReleaseAsset release;
    CHECK(ParseLatestRelease(body, "a.nro", &release) == ReleaseParseStatus::UntrustedUrl);
    CHECK(release.downloadUrl.empty());
}

void testParseRejectsMalformedBodies() {
    ReleaseAsset release;
    CHECK(ParseLatestRelease("", "a.nro", &release) == ReleaseParseStatus::Malformed);
    CHECK(ParseLatestRelease("not json", "a.nro", &release) == ReleaseParseStatus::Malformed);
    CHECK(ParseLatestRelease("[]", "a.nro", &release) == ReleaseParseStatus::Malformed);
    CHECK(ParseLatestRelease(R"({"assets":[]})", "a.nro", &release) == ReleaseParseStatus::Malformed);
    CHECK(ParseLatestRelease(R"({"tag_name":7})", "a.nro", &release) == ReleaseParseStatus::Malformed);
}

void testInstallReplacesTargetAndRemovesBackup() {
    std::string dir = MakeScratchDirectory("replace");
    std::string target = dir + "/game.nro";
    std::string part = target + ".part";
    WriteFile(target, FakeNro("old"));
    WriteFile(part, FakeNro("new"));

    CHECK(InstallDownloadedNro(part, target, FakeNro("new").size()) == InstallStatus::Installed);
    CHECK(ReadFile(target) == FakeNro("new"));
    CHECK(!Exists(part));
    CHECK(!Exists(target + ".old"));
}

void testInstallWorksWhenTargetIsMissing() {
    std::string dir = MakeScratchDirectory("missing_target");
    std::string target = dir + "/game.nro";
    WriteFile(target + ".part", FakeNro("new"));

    CHECK(InstallDownloadedNro(target + ".part", target, 0) == InstallStatus::Installed);
    CHECK(ReadFile(target) == FakeNro("new"));
}

void testInstallRejectsBadDownloadsAndKeepsTarget() {
    std::string dir = MakeScratchDirectory("rejects");
    std::string target = dir + "/game.nro";
    std::string part = target + ".part";
    WriteFile(target, FakeNro("old"));

    CHECK(InstallDownloadedNro(part, target, 0) == InstallStatus::PartMissing);

    WriteFile(part, "");
    CHECK(InstallDownloadedNro(part, target, 0) == InstallStatus::PartEmpty);

    WriteFile(part, FakeNro("new"));
    CHECK(InstallDownloadedNro(part, target, 9999) == InstallStatus::SizeMismatch);

    WriteFile(part, "<html>rate limited</html> padding padding");
    CHECK(InstallDownloadedNro(part, target, 0) == InstallStatus::NotNroImage);

    CHECK(ReadFile(target) == FakeNro("old"));
}

void testInstallKeepsTargetWhenBackupCannotBeMade() {
    std::string dir = MakeScratchDirectory("backup_fails");
    std::string target = dir + "/game.nro";
    std::string part = target + ".part";
    WriteFile(target, FakeNro("old"));
    WriteFile(part, FakeNro("new"));
    std::filesystem::create_directories(target + ".old/blocker");

    CHECK(InstallDownloadedNro(part, target, 0) == InstallStatus::BackupFailed);
    CHECK(ReadFile(target) == FakeNro("old"));
    CHECK(Exists(part));
}

} // namespace

#ifdef UPDATER_TESTS_HAVE_ZIP
int RunZipInstallTests();
#endif

int main() {
    testTrustedUrlAcceptsOnlyGithubHttps();
    testParseFindsNamedAssetWithSize();
    testTrustedApiUrlAcceptsOnlyGithubApiHttps();
    testParseLeavesApiUrlEmptyWhenUntrusted();
    testParseKeepsTagWhenAssetIsMissing();
    testParseRejectsUntrustedUrl();
    testParseRejectsMalformedBodies();
    testInstallReplacesTargetAndRemovesBackup();
    testInstallWorksWhenTargetIsMissing();
    testInstallRejectsBadDownloadsAndKeepsTarget();
    testInstallKeepsTargetWhenBackupCannotBeMade();

#ifdef UPDATER_TESTS_HAVE_ZIP
    s_failures += RunZipInstallTests();
#endif

    if (s_failures != 0) {
        std::cerr << s_failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "updater tests passed\n";
    return 0;
}
