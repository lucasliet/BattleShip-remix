#include "bridge/reloc_size_cache.h"

#include <ship/Context.h>
#include <ship/resource/ResourceManager.h>
#include <ship/resource/archive/Archive.h>
#include <ship/resource/archive/ArchiveManager.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

namespace {

constexpr const char *kCacheFileName = "reloc_sizes.cache";

/** @return the cache file path in the app directory. */
std::string portRelocSizeCachePath()
{
	return Ship::Context::GetPathRelativeToAppDirectory(kCacheFileName);
}

/**
 * Builds the key the stored sizes are valid for: the asset recipe hash plus every mounted archive's path
 * and byte size, so a new .o2r or an added or removed mod invalidates the file.
 * @return the key, or an empty string when the archives cannot be listed.
 */
std::string portRelocSizeCacheKey()
{
	auto context = Ship::Context::GetInstance();
	auto resources = context ? context->GetResourceManager() : nullptr;
	auto archives = resources ? resources->GetArchiveManager()->GetArchives() : nullptr;
	if (!archives)
	{
		return {};
	}
	std::ostringstream key;
	key << SSB64_ASSET_RECIPE_HASH;
	for (const auto &archive : *archives)
	{
		std::error_code ec;
		const auto size = fs::file_size(archive->GetPath(), ec);
		key << '|' << archive->GetPath() << ':' << (ec ? 0 : size);
	}
	return key.str();
}

} /* namespace */

bool portRelocSizeCacheLoad(RelocFileSizeMap &sizes)
{
	const std::string key = portRelocSizeCacheKey();
	std::ifstream in(portRelocSizeCachePath());
	std::string stored_key;
	if (key.empty() || !in || !std::getline(in, stored_key) || stored_key != key)
	{
		return false;
	}
	uint32_t file_id;
	size_t size;
	while (in >> file_id >> size)
	{
		sizes[file_id] = size;
	}
	return !sizes.empty();
}

void portRelocSizeCacheSave(const RelocFileSizeMap &sizes)
{
	const std::string key = portRelocSizeCacheKey();
	if (key.empty())
	{
		return;
	}
	std::ofstream out(portRelocSizeCachePath(), std::ios::trunc);
	out << key << '\n';
	for (const auto &[file_id, size] : sizes)
	{
		out << file_id << ' ' << size << '\n';
	}
	if (!out)
	{
		SPDLOG_WARN("Could not write the reloc size cache {}; the stage select will size its files again.",
		            portRelocSizeCachePath());
	}
}
