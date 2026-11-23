#include "save_convert.hpp"

#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace nepconvert {

namespace {

std::vector<uint8_t> readAllBytes(const std::string& path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) {
        throw std::runtime_error("could not open '" + path + "' for reading");
    }
    std::streamsize size = in.tellg();
    if (size < 0) {
        throw std::runtime_error("could not determine size of '" + path + "'");
    }
    in.seekg(0);
    std::vector<uint8_t> data(static_cast<size_t>(size));
    if (size > 0 && !in.read(reinterpret_cast<char*>(data.data()), size)) {
        throw std::runtime_error("failed reading '" + path + "'");
    }
    return data;
}

std::vector<uint8_t> buildPlaceholderHeader(std::optional<int> slotNumber) {
    std::vector<uint8_t> header(kPcHeaderSize, 0);
    std::memcpy(header.data(), "Neptunia", 8);

    std::ostringstream text;
    if (slotNumber.has_value()) {
        char slotBuf[16];
        std::snprintf(slotBuf, sizeof(slotBuf), "%02d", *slotNumber);
        text << "Save Data  -" << slotBuf << "-\n";
    } else {
        text << "Save Data\n";
    }
    text << "(converted with NepConvert <3)\n\nhttps://github.com/NepTools/NepConvert";

    const std::string s = text.str();
    constexpr size_t kTextOffset = 0x400;
    if (kTextOffset + s.size() < kPcHeaderSize) {
        std::memcpy(header.data() + kTextOffset, s.data(), s.size());
    }
    return header;
}

} // namespace

SaveFile loadSaveFile(const std::string& path) {
    std::vector<uint8_t> data = readAllBytes(path);
    if (data.size() == kPcSize) {
        return SaveFile{Platform::Pc, std::move(data)};
    }
    if (data.size() == kVitaSize) {
        return SaveFile{Platform::Vita, std::move(data)};
    }
    throw std::runtime_error(
        "'" + path + "' is " + std::to_string(data.size()) +
        " bytes; expected " + std::to_string(kPcSize) + " (PC) or " +
        std::to_string(kVitaSize) + " (Vita)");
}

std::vector<uint8_t> pcToVita(const std::vector<uint8_t>& pcData) {
    if (pcData.size() != kPcSize) {
        throw std::runtime_error("pcToVita: expected " +
                                  std::to_string(kPcSize) + " bytes, got " +
                                  std::to_string(pcData.size()));
    }
    return std::vector<uint8_t>(pcData.begin() + kPcHeaderSize, pcData.end());
}

std::vector<uint8_t> vitaToPc(const std::vector<uint8_t>& vitaData,
                               std::optional<int> slotNumber) {
    if (vitaData.size() != kVitaSize) {
        throw std::runtime_error("vitaToPc: expected " +
                                  std::to_string(kVitaSize) + " bytes, got " +
                                  std::to_string(vitaData.size()));
    }
    std::vector<uint8_t> result = buildPlaceholderHeader(slotNumber);
    result.insert(result.end(), vitaData.begin(), vitaData.end());
    return result;
}

std::optional<int> parsePcStorySlot(const std::string& basename) {
    static const std::string prefix = "nep_story";
    static const std::string suffix = ".dat";
    if (basename.size() <= prefix.size() + suffix.size()) return std::nullopt;
    if (basename.compare(0, prefix.size(), prefix) != 0) return std::nullopt;
    if (basename.compare(basename.size() - suffix.size(), suffix.size(), suffix) != 0)
        return std::nullopt;

    std::string digits = basename.substr(
        prefix.size(), basename.size() - prefix.size() - suffix.size());
    if (digits.empty()) return std::nullopt;
    for (char c : digits) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return std::nullopt;
    }
    return std::stoi(digits);
}

std::optional<std::string> deriveOutputFilename(const std::string& inputBasename) {
    if (inputBasename == "nepd_sys.dat") {
        return "nep_sys01.dat";
    }
    if (inputBasename.rfind("nep_sys", 0) == 0 &&
        inputBasename.size() > 4 &&
        inputBasename.compare(inputBasename.size() - 4, 4, ".dat") == 0) {
        return "nepd_sys.dat";
    }
    if (auto pcSlot = parsePcStorySlot(inputBasename)) {
        int vitaSlot = *pcSlot - 1;
        if (vitaSlot < 0) return std::nullopt;
        char buf[32];
        std::snprintf(buf, sizeof(buf), "nepd_%02d.dat", vitaSlot);
        return std::string(buf);
    }
    static const std::string vitaPrefix = "nepd_";
    static const std::string suffix = ".dat";
    if (inputBasename.rfind(vitaPrefix, 0) == 0 &&
        inputBasename.size() > vitaPrefix.size() + suffix.size() &&
        inputBasename.compare(inputBasename.size() - suffix.size(), suffix.size(), suffix) == 0) {
        std::string digits = inputBasename.substr(
            vitaPrefix.size(),
            inputBasename.size() - vitaPrefix.size() - suffix.size());
        bool allDigits = !digits.empty();
        for (char c : digits) {
            if (!std::isdigit(static_cast<unsigned char>(c))) { allDigits = false; break; }
        }
        if (allDigits) {
            int pcSlot = std::stoi(digits) + 1;
            char buf[32];
            std::snprintf(buf, sizeof(buf), "nep_story%02d.dat", pcSlot);
            return std::string(buf);
        }
    }
    return std::nullopt;
}

bool hasNeptuniaMagic(const std::vector<uint8_t>& pcData) {
    return pcData.size() >= 8 && std::memcmp(pcData.data(), "Neptunia", 8) == 0;
}

bool hasVersion100Field(const std::vector<uint8_t>& coreData) {
    if (coreData.size() < 4) return false;
    uint32_t v;
    std::memcpy(&v, coreData.data(), 4);
    return v == 100;
}

} // namespace nepconvert
