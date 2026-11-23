#include "save_convert.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;
using namespace nepconvert;

namespace {

void printUsage(const char* argv0) {
    std::cerr << "Usage:\n"
              << "  " << argv0 << " <input save file> <output save file>";
}

std::optional<int> parseVitaStorySlot(const std::string& basename) {
    static const std::string prefix = "nepd_";
    static const std::string suffix = ".dat";
    if (basename.size() <= prefix.size() + suffix.size()) return std::nullopt;
    if (basename.compare(0, prefix.size(), prefix) != 0) return std::nullopt;
    std::string digits = basename.substr(
        prefix.size(), basename.size() - prefix.size() - suffix.size());
    if (digits.empty()) return std::nullopt;
    for (char c : digits) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return std::nullopt;
    }
    return std::stoi(digits);
}

void writeAllBytes(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("could not open '" + path + "' for writing");
    }
    if (!data.empty()) {
        out.write(reinterpret_cast<const char*>(data.data()),
                   static_cast<std::streamsize>(data.size()));
    }
    if (!out) {
        throw std::runtime_error("failed writing '" + path + "'");
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        printUsage(argv[0]);
        return argc < 2 ? 1 : 0;
    }

    const fs::path inputPath = argv[1];
    const std::string inputBasename = inputPath.filename().string();

    try {
        SaveFile input = loadSaveFile(inputPath.string());

        fs::path outputPath;
        if (argc == 3) {
            outputPath = argv[2];
        } else {
            auto derived = deriveOutputFilename(inputBasename);
            if (!derived) {
                std::cerr << "Error: couldn't guess an output filename for '"
                          << inputBasename
                          << "'. Please pass the output path explicitly.\n";
                return 1;
            }
            outputPath = inputPath.parent_path() / *derived;
        }

        std::vector<uint8_t> outputData;
        if (input.platform == Platform::Pc) {
            if (!hasNeptuniaMagic(input.data)) {
                std::cerr << "Warning: input doesn't start with the expected "
                             "'Neptunia' magic; converting anyway.\n";
            }
            outputData = pcToVita(input.data);
            std::cout << "PC -> Vita: '" << inputPath.string() << "' ("
                      << input.data.size() << " bytes) -> '"
                      << outputPath.string() << "' (" << outputData.size()
                      << " bytes)\n";
        } else {
            if (!hasVersion100Field(input.data)) {
                std::cerr << "Warning: input doesn't start with the expected "
                             "version field; converting anyway.\n";
            }
            std::optional<int> pcSlot;
            if (auto vitaSlot = parseVitaStorySlot(inputBasename)) {
                pcSlot = *vitaSlot + 1;
            }
            outputData = vitaToPc(input.data, pcSlot);
            std::cout << "Vita -> PC: '" << inputPath.string() << "' ("
                      << input.data.size() << " bytes) -> '"
                      << outputPath.string() << "' (" << outputData.size()
                      << " bytes)\n"
                      << "Note: the PC save-select screen's displayed play "
                         "time/level/quest% is a placeholder until you save again!";
        }

        writeAllBytes(outputPath.string(), outputData);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
