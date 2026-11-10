/*
 * First-run extraction with Torch linked into the game. Windows uses it because Smart App Control
 * refuses to start the unsigned torch.exe sidecar (CreateProcess error 4551); see
 * docs/bugs/windows_smart_app_control_manifest_2026-11-10.md.
 */
#ifdef SSB64_TORCH_IN_PROCESS

#include "torch_in_process.h"

#include "Companion.h"
#include "port_log.h"

#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

namespace ssb64 {
namespace {

/**
 * Copies spdlog's default logger output into the extraction log while alive. The stand-in logger
 * keeps the previous logger's name and sinks, so the game log and spdlog::get() keep working, and
 * the previous logger and level come back on destruction (Torch's config.yml sets a global level).
 */
class TorchLogRedirect {
  public:
    explicit TorchLogRedirect(const std::string& logPath)
        : previousLogger_(spdlog::default_logger()), previousLevel_(spdlog::get_level()) {
        if (logPath.empty()) {
            return;
        }
        file_.open(std::filesystem::path(logPath), std::ios::app);
        if (!file_) {
            port_log("first_run: WARNING could not open %s for Torch's log\n", logPath.c_str());
            return;
        }
        std::vector<spdlog::sink_ptr> sinks = previousLogger_->sinks();
        sinks.push_back(std::make_shared<spdlog::sinks::ostream_sink_mt>(file_, true));
        auto logger = std::make_shared<spdlog::logger>(previousLogger_->name(), sinks.begin(), sinks.end());
        logger->set_level(previousLogger_->level());
        spdlog::set_default_logger(logger);
    }

    ~TorchLogRedirect() {
        spdlog::set_default_logger(previousLogger_);
        spdlog::set_level(previousLevel_);
    }

    TorchLogRedirect(const TorchLogRedirect&) = delete;
    TorchLogRedirect& operator=(const TorchLogRedirect&) = delete;

  private:
    std::shared_ptr<spdlog::logger> previousLogger_;
    spdlog::level::level_enum previousLevel_;
    std::ofstream file_;
};

/**
 * Runs one Torch O2R export, the same as `torch o2r <rom> -s <configDir> -d <outputDir>`.
 * @return Torch's output path; empty when Torch stopped before configuring its output
 */
std::string RunCompanion(const std::string& rom, const std::string& configDir, const std::string& outputDir) {
    auto companion = std::make_unique<Companion>(std::filesystem::path(rom), ArchiveType::O2R, false, configDir,
                                                 outputDir);
    Companion::Instance = companion.get();
    struct InstanceReset {
        ~InstanceReset() { Companion::Instance = nullptr; }
    } instanceReset;
    companion->Init(ExportType::Binary);
    return companion->GetOutputPath();
}

} // namespace

bool RunTorchInProcess(const std::string& rom, const std::string& configDir, const std::string& outputDir,
                       const std::string& logPath, std::string& error) {
    TorchLogRedirect logRedirect(logPath);
    try {
        if (RunCompanion(rom, configDir, outputDir).empty()) {
            error = "Torch stopped before writing the archive: the ROM is not a supported dump, or config.yml "
                    "and yamls/ are missing from " + configDir;
            return false;
        }
        return true;
    } catch (const std::exception& exception) {
        error = std::string("Torch failed: ") + exception.what();
    } catch (...) {
        error = "Torch failed with an unknown error";
    }
    return false;
}

} // namespace ssb64

#endif
