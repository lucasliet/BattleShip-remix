/**
 * remix_roster.cpp - loads the Smash Remix fighter blob ("remix/fighters",
 * format in docs/remix/torch_factories.md, "Contrato dos blobs") and registers its synth fighters.
 *
 * The blob is parsed here into plain values. The FTData itself is built on the
 * decomp side (port_fighter_synth_ft_create in decomp/src/ft/ftport.c), since the
 * C++ port side cannot include the decomp headers. The roster table
 * (kRemixRoster, REMIX_ID_TO_PORT_FKIND) comes from remix_roster_data.h. Until
 * that header exists the loader is a stub that logs a warning and returns false.
 */

#include "remix_roster.h"

#if __has_include("remix_roster_data.h")

#include <ship/Context.h>
#include <ship/resource/ResourceManager.h>
#include <ship/resource/type/Blob.h>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "fighter_registry.h"
#include "remix/remix_action_bind.h"
#include "remix/remix_action_layout.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include "remix/remix_synth_ft.h"
#include "remix_roster_data.h"

namespace {

constexpr uint32_t kBlobMagic = 0x46584D52u;
constexpr uint32_t kBlobVersion = 1u;
constexpr size_t kHeaderSize = 16;
constexpr size_t kRecordSize = 0x80;
constexpr size_t kMotionDescSize = 12;
constexpr size_t kRosterCount = std::size(kRemixRoster);
constexpr int kRandomRemixId = 0x1B;
constexpr int kUnusedCloneRemixId = 0x1C;
constexpr int kFirstSynthRemixId = kRandomRemixId;
constexpr int kLastSynthRemixId = 0x60;
constexpr uint8_t kVanillaFighterCount = 27;
constexpr int kFirstRelocatedAttributesFkind = 13;
constexpr int kLastRelocatedAttributesFkind = 25;
constexpr uint32_t kEntryActionRowSize = 8;
constexpr int kFirstVanillaMenuZoomFkind = 12;
constexpr int kLastVanillaMenuZoomFkind = 26;

/* Field offsets inside one 0x80-byte record (see docs/remix/torch_factories.md). */
constexpr size_t kOffFileIds = 0x04;
constexpr size_t kOffFileMainSize = 0x28;
constexpr size_t kOffOAttributes = 0x2C;
constexpr size_t kOffFileAnimSize = 0x30;
constexpr size_t kOffMainMotionCount = 0x34;
constexpr size_t kOffMainMotionOff = 0x38;
constexpr size_t kOffMenuCount = 0x3C;
constexpr size_t kOffMenuOff = 0x40;

struct RemixRecord {
    uint32_t remix_id;
    uint32_t file_ids[PORT_SYNTH_FILE_COUNT];
    uint32_t file_main_size;
    uint32_t o_attributes;
    uint32_t file_anim_size;
    uint32_t mainmotion_count;
    uint32_t mainmotion_off;
    uint32_t menu_count;
    uint32_t menu_off;
};

using FtDataPtr = std::unique_ptr<struct FTData, void (*)(struct FTData *)>;

/* Owns one synth. The descriptor points at the FTData and the name, so the
 * SynthFighter must not move; it lives behind a unique_ptr in sSynthFighters. */
struct SynthFighter {
    int remix_id = 0;
    FtDataPtr ft_data{nullptr, port_fighter_synth_ft_destroy};
    std::string name;
    FighterDescriptor descriptor{};
};

using SynthFighterPtr = std::unique_ptr<SynthFighter>;

std::vector<SynthFighterPtr> sSynthFighters;

uint32_t readLE32(const std::vector<uint8_t> &bytes, size_t offset)
{
    return static_cast<uint32_t>(bytes[offset])
         | (static_cast<uint32_t>(bytes[offset + 1]) << 8)
         | (static_cast<uint32_t>(bytes[offset + 2]) << 16)
         | (static_cast<uint32_t>(bytes[offset + 3]) << 24);
}

bool rangeFits(const std::vector<uint8_t> &bytes, uint64_t offset, uint64_t length)
{
    return offset <= bytes.size() && length <= bytes.size() - offset;
}

/* Motion arrays start right after the record table. Offsets in the records are
 * relative to this point. */
uint64_t motionRegionOffset()
{
    return kHeaderSize + uint64_t(kRosterCount) * kRecordSize;
}

std::shared_ptr<Ship::Blob> loadRemixBlob()
{
    auto ctx = Ship::Context::GetInstance();
    if (!ctx) {
        spdlog::warn("remix_roster: no Ship::Context; Remix fighters disabled");
        return nullptr;
    }
    auto resource = ctx->GetResourceManager()->LoadResource("__OTR__remix/fighters");
    auto blob = std::dynamic_pointer_cast<Ship::Blob>(resource);
    if (!blob) {
        spdlog::warn("remix_roster: resource 'remix/fighters' not found in the o2r; Remix fighters disabled");
    }
    return blob;
}

bool validateMagicAndVersion(const std::vector<uint8_t> &bytes)
{
    const uint32_t magic = readLE32(bytes, 0);
    if (magic != kBlobMagic) {
        spdlog::error("remix_roster: bad magic 0x{:08X}, expected 'RMXF' (0x{:08X})", magic, kBlobMagic);
        return false;
    }
    const uint32_t version = readLE32(bytes, 4);
    if (version != kBlobVersion) {
        spdlog::error("remix_roster: blob version {}, expected {}", version, kBlobVersion);
        return false;
    }
    return true;
}

bool validateHeader(const std::vector<uint8_t> &bytes)
{
    if (!rangeFits(bytes, 0, kHeaderSize)) {
        spdlog::error("remix_roster: blob is {} bytes, shorter than the {}-byte header", bytes.size(), kHeaderSize);
        return false;
    }
    if (!validateMagicAndVersion(bytes)) {
        return false;
    }
    const uint32_t count = readLE32(bytes, 8);
    if (count != kRosterCount) {
        spdlog::error("remix_roster: blob has {} records, expected {} (kRemixRoster size)", count, kRosterCount);
        return false;
    }
    if (!rangeFits(bytes, kHeaderSize, uint64_t(count) * kRecordSize)) {
        spdlog::error("remix_roster: {} records of {} bytes do not fit in the {}-byte blob",
                      count, kRecordSize, bytes.size());
        return false;
    }
    return true;
}

RemixRecord readRecord(const std::vector<uint8_t> &bytes, size_t index)
{
    const size_t base = kHeaderSize + index * kRecordSize;
    RemixRecord record{};
    record.remix_id = readLE32(bytes, base);
    for (size_t i = 0; i < PORT_SYNTH_FILE_COUNT; ++i) {
        record.file_ids[i] = readLE32(bytes, base + kOffFileIds + i * 4);
    }
    record.file_main_size = readLE32(bytes, base + kOffFileMainSize);
    record.o_attributes = readLE32(bytes, base + kOffOAttributes);
    record.file_anim_size = readLE32(bytes, base + kOffFileAnimSize);
    record.mainmotion_count = readLE32(bytes, base + kOffMainMotionCount);
    record.mainmotion_off = readLE32(bytes, base + kOffMainMotionOff);
    record.menu_count = readLE32(bytes, base + kOffMenuCount);
    record.menu_off = readLE32(bytes, base + kOffMenuOff);
    return record;
}

/* Reads `count` motion rows (12 bytes each in the blob) into plain structs. */
bool readMotionArray(const std::vector<uint8_t> &bytes, uint64_t offset, uint32_t count,
                     uint32_t remixId, std::vector<PortSynthMotion> &out)
{
    out.clear();
    if (count == 0) {
        return true;
    }
    const uint64_t length = uint64_t(count) * kMotionDescSize;
    if (!rangeFits(bytes, offset, length)) {
        spdlog::error("remix_roster: remix id 0x{:02X} motion array at {} ({} rows) exceeds the {}-byte blob",
                      remixId, offset, count, bytes.size());
        return false;
    }
    out.resize(count);
    for (uint32_t i = 0; i < count; ++i) {
        const size_t at = static_cast<size_t>(offset) + i * kMotionDescSize;
        out[i].anim_file_id = readLE32(bytes, at);
        out[i].command_offset = readLE32(bytes, at + 4);
        out[i].flags = readLE32(bytes, at + 8);
    }
    return true;
}

bool readMotionArrays(const std::vector<uint8_t> &bytes, const RemixRecord &record,
                      std::vector<PortSynthMotion> &mainmotion, std::vector<PortSynthMotion> &submotion)
{
    return readMotionArray(bytes, motionRegionOffset() + record.mainmotion_off, record.mainmotion_count,
                           record.remix_id, mainmotion)
        && readMotionArray(bytes, motionRegionOffset() + record.menu_off, record.menu_count,
                           record.remix_id, submotion);
}

PortSynthFtSpec makeSpec(const RemixRecord &record, const std::vector<PortSynthMotion> &mainmotion,
                         const std::vector<PortSynthMotion> &submotion)
{
    PortSynthFtSpec spec{};
    std::memcpy(spec.file_ids, record.file_ids, sizeof(spec.file_ids));
    spec.file_main_size = record.file_main_size;
    spec.o_attributes = record.o_attributes;
    spec.file_anim_size = record.file_anim_size;
    spec.mainmotion = mainmotion.data();
    spec.mainmotion_count = record.mainmotion_count;
    spec.submotion = submotion.data();
    spec.submotion_count = record.menu_count;
    return spec;
}

/* Validates the record against the roster entry and its parent. Returns the
 * parent's fkind, or -1 after logging why the record is unusable. */
int resolveParentFkind(const RemixRecord &record, size_t index)
{
    const auto &entry = kRemixRoster[index];
    if (record.remix_id != static_cast<uint32_t>(index)) {
        spdlog::error("remix_roster: record {} claims remix id 0x{:02X}", index, record.remix_id);
        return -1;
    }
    if (static_cast<size_t>(entry.remix_id) != index) {
        spdlog::error("remix_roster: kRemixRoster[{}] has remix_id {}, expected {}", index, entry.remix_id, index);
        return -1;
    }
    if (entry.parent_remix_id >= kVanillaFighterCount) {
        spdlog::error("remix_roster: remix id 0x{:02X} parent 0x{:02X} is not a vanilla fighter (synth parents unsupported)",
                      index, entry.parent_remix_id);
        return -1;
    }
    /* Vanilla Remix ids 0x00..0x1A equal their port fkind. */
    const int parentFkind = entry.parent_remix_id;
    if (port_fighter_descriptor(parentFkind) == nullptr) {
        spdlog::error("remix_roster: parent fkind {} for remix id 0x{:02X} is not registered", parentFkind, index);
        return -1;
    }
    return parentFkind;
}

/* Character.crowd_chant_fgm.table (Character.asm get_crowd_chant_fgm_): the FGM the crowd chants for a fighter when the
 * rally starts, one halfword per fighter id. The polygon rows hold 0x02B7, "no voice". The VS results winner line
 * (name, "WINS!" x, announcer voice, emblem) is read from the Remix tables by mn/mnvsmode/mnremixresults.c. */
constexpr uint32_t kCrowdChantEntrySize = 2;

/* Crowd chant of a Remix fighter, or -1 when the table is not in the image. */
int readCrowdChantFgm(int remixId)
{
    const uint32_t fgm = remix_ram_image_read_half(REMIX_SYM_Character__crowd_chant_fgm__table
                                                   + static_cast<uint32_t>(remixId) * kCrowdChantEntrySize);
    return (fgm != 0) ? static_cast<int>(fgm) : -1;
}

/* Gives the synth its own crowd chant instead of its parent's. The registry field is named after the caller,
 * ftPublicTryStartCall (ft/ftpublic.c), which reads it as the FGM of the chant. */
void bindCrowdChant(SynthFighter &synth, int remixId)
{
    const int fgm = readCrowdChantFgm(remixId);
    if (fgm < 0) {
        spdlog::warn("remix_roster: no crowd chant FGM for remix id 0x{:02X}, keeping the parent's", remixId);
        return;
    }
    synth.descriptor.public_call_fgm = fgm;
}

/* Replaces the parent's special statuses with the synth's own Remix action
 * array. Its first (count - add_actions) entries are the parent's slots. */
void bindSpecials(SynthFighter &synth, int remixId, int parentFkind)
{
    const int count = kRemixActionCounts[remixId];
    const int parentActionCount = count - kRemixRoster[static_cast<size_t>(remixId)].add_actions;
    ftRemixBindSpecials(remixId, parentFkind, parentActionCount, &synth.descriptor);
    ftRemixBindDescriptorHooks(remixId, &synth.descriptor);
}

/* Remix keeps one row of two entry statuses per character (Character.entry_action.table); a clone starts
 * with its parent's row and may override it (Peach, Marth, Dedede...). Keeping the parent's statuses
 * plays another fighter's action on the clone's own motions. */
void bindEntryStatuses(SynthFighter &synth, int remixId)
{
    const uint32_t row = REMIX_SYM_Character__entry_action__table + static_cast<uint32_t>(remixId) * kEntryActionRowSize;
    for (uint32_t entryId = 0; entryId < 2; ++entryId) {
        const uint32_t status = remix_ram_image_read_word(row + entryId * 4u);
        if (status == 0) {
            spdlog::warn("remix_roster: no entry status {} for remix id 0x{:02X}, keeping the parent's", entryId, remixId);
            continue;
        }
        synth.descriptor.entry_appear_status[entryId] = static_cast<int>(status);
    }
}

/* Builds one synth from its Remix record. The FTData clones the parent's, so
 * fields the record does not supply keep the parent's values. The descriptor is
 * a copy of the parent's, with the Remix-specific fields replaced. */
SynthFighterPtr buildSynthFighter(const std::vector<uint8_t> &bytes, int remixId)
{
    const size_t index = static_cast<size_t>(remixId);
    const RemixRecord record = readRecord(bytes, index);
    const int parentFkind = resolveParentFkind(record, index);
    if (parentFkind < 0) {
        return nullptr;
    }

    std::vector<PortSynthMotion> mainmotion;
    std::vector<PortSynthMotion> submotion;
    if (!readMotionArrays(bytes, record, mainmotion, submotion)) {
        return nullptr;
    }

    const PortSynthFtSpec spec = makeSpec(record, mainmotion, submotion);
    auto synth = std::make_unique<SynthFighter>();
    synth->remix_id = remixId;
    synth->ft_data.reset(port_fighter_synth_ft_create(parentFkind, &spec));
    if (!synth->ft_data) {
        spdlog::error("remix_roster: could not build FTData for remix id 0x{:02X} (parent fkind {})",
                      remixId, parentFkind);
        return nullptr;
    }

    const FighterDescriptor *parent = port_fighter_descriptor(parentFkind);
    synth->name = std::string(kRemixRoster[index].name);
    synth->descriptor = *parent;
    synth->descriptor.parent_fkind = parentFkind;
    synth->descriptor.scale = kRemixRoster[index].menu_zoom;
    synth->descriptor.ft_data = synth->ft_data.get();
    bindCrowdChant(*synth, remixId);
    bindSpecials(*synth, remixId, parentFkind);
    bindEntryStatuses(*synth, remixId);
    return synth;
}

void registerSynth(const SynthFighter &synth)
{
    port_fighter_register(REMIX_ID_TO_PORT_FKIND(synth.remix_id), &synth.descriptor);
}

/* Remix rebuilt the main files of the polygon and Metal Mario fighters with the
 * attributes block at a different offset; the vanilla FTData still points at the old one. */
void patchVanillaAttributeOffsets(const std::vector<uint8_t> &bytes)
{
    for (int fkind = kFirstRelocatedAttributesFkind; fkind <= kLastRelocatedAttributesFkind; ++fkind) {
        const RemixRecord record = readRecord(bytes, static_cast<size_t>(fkind));
        if (!port_fighter_set_attributes_offset(fkind, record.o_attributes)) {
            spdlog::warn("remix_roster: vanilla fkind {} has no FTData to patch", fkind);
        }
    }
}

/* Character.asm:3573-3592: Metal Mario and the polygons take the menu motions 1 to 13 (CSS pose, victory poses, claps...) of
 * their base fighter, and Giant DK uses DK's whole menu array. Vanilla has empty rows there (NLuigi, NPurin and Giant DK have
 * a single row, so the results screen read past the end of the array and crashed for Giant DK). */
void patchVanillaMenuMotions()
{
    constexpr int kMarioFkind = 0;
    constexpr int kDonkeyFkind = 2;
    constexpr int kMetalFkind = 13;
    constexpr int kFirstPolygonFkind = 14;
    constexpr int kLastPolygonFkind = 25;
    constexpr int kGiantDonkeyFkind = 26;
    constexpr uint32_t kFirstCopiedMotion = 1;
    constexpr uint32_t kCopiedMotionCount = 13;

    bool ok = port_fighter_copy_menu_motions(kMetalFkind, kMarioFkind, kFirstCopiedMotion, kCopiedMotionCount) != 0;
    for (int fkind = kFirstPolygonFkind; fkind <= kLastPolygonFkind; ++fkind) {
        ok = (port_fighter_copy_menu_motions(fkind, fkind - kFirstPolygonFkind, kFirstCopiedMotion, kCopiedMotionCount) != 0) && ok;
    }
    ok = (port_fighter_share_menu_motions(kGiantDonkeyFkind, kDonkeyFkind) != 0) && ok;
    if (!ok) {
        spdlog::error("remix_roster: the menu motions of Metal Mario, the polygons or Giant DK were not copied (see the port log)");
    }
}

/* The vanilla descriptors of Master Hand, Metal Mario, the polygon team and Giant DK carry a neutral
 * 1.0 panel scale; Remix gives each of them its own entry in Character.menu_zoom.table. */
void patchVanillaMenuZoom()
{
    for (int fkind = kFirstVanillaMenuZoomFkind; fkind <= kLastVanillaMenuZoomFkind; ++fkind) {
        const FighterDescriptor *vanilla = port_fighter_descriptor(fkind);
        if (vanilla == nullptr) {
            continue;
        }
        FighterDescriptor patched = *vanilla;
        patched.scale = kRemixRoster[static_cast<size_t>(fkind)].menu_zoom;
        port_fighter_register(fkind, &patched);
    }
}

/* Kirby's copy abilities are extra motions and actions of Kirby's own rows in
 * Remix: its motion array grows past the vanilla rows and its action array
 * gains the copy abilities' statuses. */
bool bindKirbyCopyActions(const std::vector<uint8_t> &bytes)
{
    constexpr int kKirbyFkind = 8;
    const FighterDescriptor *vanilla = port_fighter_descriptor(kKirbyFkind);
    if (vanilla == nullptr) {
        spdlog::error("remix_roster: Kirby is not registered");
        return false;
    }

    const RemixRecord record = readRecord(bytes, static_cast<size_t>(kKirbyFkind));
    std::vector<PortSynthMotion> mainmotion;
    std::vector<PortSynthMotion> submotion;
    if (!readMotionArrays(bytes, record, mainmotion, submotion)) {
        return false;
    }
    if (port_fighter_extend_mainmotion(kKirbyFkind, static_cast<uint32_t>(ftRemixKirbyVanillaMotionCount()),
                                       mainmotion.data(), record.mainmotion_count) == 0) {
        spdlog::error("remix_roster: Kirby has {} Remix motions, expected more than {}", record.mainmotion_count,
                      ftRemixKirbyVanillaMotionCount());
        return false;
    }

    FighterDescriptor kirby = *vanilla;
    ftRemixBindKirbyActions(&kirby);
    port_fighter_register(kKirbyFkind, &kirby);
    return true;
}

} /* namespace */

bool remix_roster_load()
{
    if (!sSynthFighters.empty()) {
        return true;
    }
    auto blob = loadRemixBlob();
    if (!blob || !validateHeader(blob->Data)) {
        return false;
    }
    patchVanillaAttributeOffsets(blob->Data);
    patchVanillaMenuZoom();
    patchVanillaMenuMotions();

    std::vector<SynthFighterPtr> built;
    for (int remixId = kFirstSynthRemixId; remixId <= kLastSynthRemixId; ++remixId) {
        if (remixId == kUnusedCloneRemixId) {
            continue;
        }
        auto synth = buildSynthFighter(blob->Data, remixId);
        if (!synth) {
            return false;
        }
        built.push_back(std::move(synth));
    }

    for (const SynthFighterPtr &synth : built) {
        registerSynth(*synth);
    }
    sSynthFighters = std::move(built);
    if (!bindKirbyCopyActions(blob->Data)) {
        return false;
    }
    spdlog::info("remix_roster: registered {} synth fighters (remix ids 0x{:02X}..0x{:02X}, 0x{:02X} skipped)",
                 sSynthFighters.size(), kFirstSynthRemixId, kLastSynthRemixId, kUnusedCloneRemixId);
    return true;
}

#else /* remix_roster_data.h not generated yet */

#include <spdlog/spdlog.h>

bool remix_roster_load()
{
    spdlog::warn("remix_roster: remix_roster_data.h is missing; Remix fighters disabled");
    return false;
}

#endif
