/**
 * remix_ram_image.cpp - see remix_ram_image.h.
 */

#include "remix_ram_image.h"

#include <ship/Context.h>
#include <ship/resource/ResourceManager.h>
#include <ship/resource/type/Blob.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "resource/RelocPointerTable.h"

namespace {

constexpr uint32_t kRamImageBase = 0x80400000u;
constexpr const char *kRamImagePath = "__OTR__remix/ram_image";

/* Native-order copy of the image; its size is the blob's size rounded down to whole words. */
std::vector<uint32_t> sRamImageWords;

uint32_t readBE32(const std::vector<uint8_t> &bytes, size_t at)
{
    return (uint32_t(bytes[at]) << 24) | (uint32_t(bytes[at + 1]) << 16) | (uint32_t(bytes[at + 2]) << 8) |
           uint32_t(bytes[at + 3]);
}

std::shared_ptr<Ship::Blob> loadRamImageBlob()
{
    auto ctx = Ship::Context::GetInstance();
    if (!ctx) {
        return nullptr;
    }
    return std::dynamic_pointer_cast<Ship::Blob>(ctx->GetResourceManager()->LoadResource(kRamImagePath));
}

extern "C" void portRelocRegisterExternalRange(void *base, size_t size, const char *label);

void *resolveFallback(uint32_t value)
{
    return remix_ram_image_resolve(value);
}

} /* namespace */

int remix_ram_image_load(void)
{
    const auto blob = loadRamImageBlob();
    if (!blob) {
        spdlog::warn("remix_ram_image: resource 'remix/ram_image' not found in the o2r; "
                     "Remix motion scripts at 0x{:08X}+ will not resolve",
                     kRamImageBase);
        return 0;
    }
    const std::vector<uint8_t> &bytes = blob->Data;
    const size_t wordCount = bytes.size() / sizeof(uint32_t);
    sRamImageWords.resize(wordCount);
    for (size_t i = 0; i < wordCount; ++i) {
        sRamImageWords[i] = readBE32(bytes, i * sizeof(uint32_t));
    }
    portRelocSetFallbackResolver(resolveFallback);
    spdlog::info("remix_ram_image: {} bytes mapped at 0x{:08X}", wordCount * sizeof(uint32_t), kRamImageBase);
    return 1;
}

void *remix_ram_image_make_file(uint32_t start, uint32_t size, void *dst)
{
    const uint32_t *src = static_cast<const uint32_t *>(remix_ram_image_resolve(start));
    uint32_t *words = static_cast<uint32_t *>(dst);
    const uint32_t wordCount = size / sizeof(uint32_t);

    if ((src == nullptr) || (dst == nullptr) || (remix_ram_image_resolve(start + size - sizeof(uint32_t)) == nullptr)) {
        spdlog::error("remix_ram_image: region 0x{:08X}+0x{:X} is outside the image", start, size);
        return nullptr;
    }
    for (uint32_t i = 0; i < wordCount; ++i) {
        const uint32_t value = src[i];
        const bool isInternal = (value >= start) && (value < (start + size)) && ((value & 3u) == 0);

        words[i] = isInternal ? portRelocRegisterPointer(&words[(value - start) / sizeof(uint32_t)]) : value;
    }
    portRelocRegisterExternalRange(dst, wordCount * sizeof(uint32_t), "remix_ram_image_file");
    return dst;
}

void *remix_ram_image_resolve(uint32_t ram_addr)
{
    if (ram_addr < kRamImageBase) {
        return nullptr;
    }
    const size_t byteOffset = ram_addr - kRamImageBase;
    if (byteOffset >= sRamImageWords.size() * sizeof(uint32_t)) {
        return nullptr;
    }
    return reinterpret_cast<uint8_t *>(sRamImageWords.data()) + byteOffset;
}
