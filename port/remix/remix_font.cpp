/**
 * remix_font.cpp - see remix_font.h.
 */

#include "remix_font.h"

#ifdef REGION_REMIX

#include <ship/Context.h>
#include <ship/resource/ResourceManager.h>
#include <ship/window/Window.h>
#include <ship/window/gui/Gui.h>
#include <ship/window/gui/resource/GuiTexture.h>
#include <spdlog/spdlog.h>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

#include "remix_font_decode.h"
#include "remix_ram_image.h"
#include "remix_symbols.h"
#include "resource/RelocFile.h"

namespace {

constexpr const char *kFontResource = "reloc_misc/file_2581";
constexpr const char *kTextureName = "RemixHudFont";
constexpr int kGlyphColumns = 16;
constexpr int kGlyphRows = 6;
constexpr int kCellSize = REMIX_FONT_GLYPH_SIZE + 2;
constexpr int kAtlasWidth = kGlyphColumns * kCellSize;
constexpr int kAtlasHeight = kGlyphRows * kCellSize;
constexpr int kSpaceCharacter = 0x20;
constexpr float kSpaceAdvance = 4.0f;
constexpr uint32_t kWidthEntrySize = 8;

enum class FontState { NotLoaded, Ready, Failed };

FontState sState = FontState::NotLoaded;
ImTextureID sTexture = nullptr;
std::array<uint8_t, REMIX_FONT_LAST_CHARACTER + 1> sWidths{};

bool CharacterHasGlyph(int character) {
    return character >= REMIX_FONT_FIRST_CHARACTER && character <= REMIX_FONT_LAST_CHARACTER;
}

void ReadWidths() {
    for (int character = kSpaceCharacter; character <= REMIX_FONT_LAST_CHARACTER; character++) {
        const uint32_t entry = REMIX_SYM_Render__character_offsets_custom +
                               kWidthEntrySize * static_cast<uint32_t>(character - kSpaceCharacter);
        sWidths[character] = static_cast<uint8_t>(remix_ram_image_read_byte(entry));
    }
}

void CopyGlyphToAtlas(const uint8_t *glyph, int character, uint8_t *atlas) {
    const int index = character - REMIX_FONT_FIRST_CHARACTER;
    const int originX = (index % kGlyphColumns) * kCellSize + 1;
    const int originY = (index / kGlyphColumns) * kCellSize + 1;

    for (int row = 0; row < REMIX_FONT_GLYPH_SIZE; row++) {
        std::memcpy(atlas + (static_cast<size_t>(originY + row) * kAtlasWidth + originX) * 4,
                    glyph + static_cast<size_t>(row) * REMIX_FONT_GLYPH_SIZE * 4, REMIX_FONT_GLYPH_SIZE * 4);
    }
}

/** Decodes every glyph into a malloc'd RGBA atlas that the GuiTexture frees. Returns nullptr when a glyph fails. */
uint8_t *BuildAtlas(const std::vector<uint8_t> &fontFile) {
    auto *atlas = static_cast<uint8_t *>(std::calloc(static_cast<size_t>(kAtlasWidth) * kAtlasHeight, 4));
    uint8_t glyph[REMIX_FONT_GLYPH_RGBA_BYTES];

    if (atlas == nullptr) {
        return nullptr;
    }
    for (int character = REMIX_FONT_FIRST_CHARACTER; character <= REMIX_FONT_LAST_CHARACTER; character++) {
        if (remix_font_decode_glyph(fontFile.data(), fontFile.size(), character, glyph) == 0) {
            spdlog::error("remix_font: glyph 0x{:02X} does not decode ({} byte font file)", character, fontFile.size());
            std::free(atlas);
            return nullptr;
        }
        CopyGlyphToAtlas(glyph, character, atlas);
    }
    return atlas;
}

std::shared_ptr<RelocFile> LoadFontFile() {
    auto context = Ship::Context::GetInstance();

    if (!context) {
        return nullptr;
    }
    return std::dynamic_pointer_cast<RelocFile>(context->GetResourceManager()->LoadResource(kFontResource));
}

bool UploadAtlas(uint8_t *atlas) {
    auto context = Ship::Context::GetInstance();
    auto texture = std::make_shared<Ship::GuiTexture>();

    texture->Data = atlas;
    texture->DataSize = static_cast<size_t>(kAtlasWidth) * kAtlasHeight * 4;
    texture->Metadata.Width = kAtlasWidth;
    texture->Metadata.Height = kAtlasHeight;
    context->GetWindow()->GetGui()->LoadTextureFromResource(kTextureName, texture);
    sTexture = context->GetWindow()->GetGui()->GetTextureByName(kTextureName);
    return sTexture != nullptr;
}

ImVec2 UvOf(int character, int column, int row) {
    const int index = character - REMIX_FONT_FIRST_CHARACTER;
    const float cellX = static_cast<float>((index % kGlyphColumns) * kCellSize + 1 + column * REMIX_FONT_GLYPH_SIZE);
    const float cellY = static_cast<float>((index / kGlyphColumns) * kCellSize + 1 + row * REMIX_FONT_GLYPH_SIZE);
    return ImVec2(cellX / kAtlasWidth, cellY / kAtlasHeight);
}

float AdvanceOf(int character, float scale) {
    if (character == kSpaceCharacter || !CharacterHasGlyph(character)) {
        return kSpaceAdvance * scale;
    }
    return (static_cast<float>(sWidths[character]) + 1.0f) * scale;
}

} // namespace

namespace RemixFont {

bool Ensure() {
    if (sState != FontState::NotLoaded) {
        return sState == FontState::Ready;
    }
    sState = FontState::Failed;
    auto file = LoadFontFile();
    if (!file) {
        spdlog::warn("remix_font: resource {} not found in the o2r; HUD text falls back to the ImGui font", kFontResource);
        return false;
    }
    ReadWidths();
    if (sWidths['A'] == 0) {
        spdlog::warn("remix_font: Remix RAM image has no glyph widths; HUD text falls back to the ImGui font");
        return false;
    }
    uint8_t *atlas = BuildAtlas(file->Data);
    if (atlas == nullptr || !UploadAtlas(atlas)) {
        return false;
    }
    sState = FontState::Ready;
    return true;
}

float Measure(const std::string &text, float scale) {
    float width = 0.0f;

    for (const char character : text) {
        width += AdvanceOf(static_cast<unsigned char>(character), scale);
    }
    return width;
}

void Draw(ImDrawList *drawList, ImVec2 position, float pixelsPerUnit, float scale, ImU32 color,
          const std::string &text) {
    const float glyphPixels = REMIX_FONT_GLYPH_SIZE * scale * pixelsPerUnit;
    float cursor = position.x;
    const float uvSize = static_cast<float>(REMIX_FONT_GLYPH_SIZE);

    for (const char raw : text) {
        const int character = static_cast<unsigned char>(raw);

        if (CharacterHasGlyph(character)) {
            const ImVec2 uvMin = UvOf(character, 0, 0);
            const ImVec2 uvMax(uvMin.x + uvSize / kAtlasWidth, uvMin.y + uvSize / kAtlasHeight);
            drawList->AddImage(sTexture, ImVec2(cursor, position.y), ImVec2(cursor + glyphPixels, position.y + glyphPixels),
                               uvMin, uvMax, color);
        }
        cursor += AdvanceOf(character, scale) * pixelsPerUnit;
    }
}

} // namespace RemixFont

#endif
