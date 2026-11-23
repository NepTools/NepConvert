#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace nepconvert {

constexpr size_t kVitaSize = 0x8000;
constexpr size_t kPcHeaderSize = 0x8000;
constexpr size_t kPcSize = kPcHeaderSize + kVitaSize;

enum class Platform { Pc, Vita };

struct SaveFile {
    Platform platform;
    std::vector<uint8_t> data;
};


SaveFile loadSaveFile(const std::string& path);
std::vector<uint8_t> pcToVita(const std::vector<uint8_t>& pcData);
std::vector<uint8_t> vitaToPc(const std::vector<uint8_t>& vitaData,
                               std::optional<int> slotNumber);
std::optional<std::string> deriveOutputFilename(const std::string& inputBasename);
std::optional<int> parsePcStorySlot(const std::string& basename);

bool hasNeptuniaMagic(const std::vector<uint8_t>& pcData);
bool hasVersion100Field(const std::vector<uint8_t>& coreData);

} // namespace nepconvert
