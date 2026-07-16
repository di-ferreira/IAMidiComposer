// dsp_cli.cpp — Audio analysis CLI tool
//
// Reads a WAV file and runs DSP analysis:
//   - Onset detection  → onset times + BPM estimation
//   - Key detection     → key, scale, confidence
//   - Chord detection   → chord progression
//
// Outputs JSON to stdout or file.
//
// Usage:
//   dsp_cli --input <file.wav> [--output <file.json>] [--verbose]

#include <aimidi/dsp/FFT.hpp>
#include <aimidi/dsp/Windows.hpp>
#include <aimidi/dsp/OnsetDetection.hpp>
#include <aimidi/dsp/KeyDetection.hpp>
#include <aimidi/dsp/ChordDetection.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace {

// ---------------------------------------------------------------------------
// WAV reader (PCM 16-bit, mono or stereo → mixed to mono)
// ---------------------------------------------------------------------------

std::vector<float> readWAV(const std::string& path, int& sampleRate, int& numChannels) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::fprintf(stderr, "Error: cannot open '%s'\n", path.c_str());
        return {};
    }

    // Read RIFF header
    char riff[4];
    uint32_t fileSize;
    char wave[4];
    file.read(riff, 4);
    file.read(reinterpret_cast<char*>(&fileSize), 4);
    file.read(wave, 4);

    if (std::memcmp(riff, "RIFF", 4) != 0 || std::memcmp(wave, "WAVE", 4) != 0) {
        std::fprintf(stderr, "Error: not a RIFF/WAVE file\n");
        return {};
    }

    // Walk chunks until we find "fmt " then "data"
    uint16_t audioFormat = 0;
    uint16_t numChannelsTemp = 0;
    uint32_t sampleRateTemp = 0;
    uint16_t bitsPerSample = 0;
    uint32_t dataSize = 0;
    bool foundFmt = false;

    char chunkId[4];
    uint32_t chunkSize;

    while (file.read(chunkId, 4) && file.read(reinterpret_cast<char*>(&chunkSize), 4)) {
        if (!foundFmt && std::memcmp(chunkId, "fmt ", 4) == 0) {
            uint16_t formatTag;
            file.read(reinterpret_cast<char*>(&formatTag), 2);
            file.read(reinterpret_cast<char*>(&numChannelsTemp), 2);
            file.read(reinterpret_cast<char*>(&sampleRateTemp), 4);

            // Skip byteRate (4) and blockAlign (2)
            file.seekg(6, std::ios::cur);

            file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

            audioFormat = formatTag;
            foundFmt = true;

            // Skip remaining fmt chunk bytes
            uint32_t remaining = chunkSize - 16;
            if (remaining > 0) {
                file.seekg(remaining, std::ios::cur);
            }
        } else if (std::memcmp(chunkId, "data", 4) == 0) {
            dataSize = chunkSize;
            break;
        } else {
            // Skip unknown chunk
            file.seekg(chunkSize, std::ios::cur);
        }
    }

    if (!foundFmt) {
        std::fprintf(stderr, "Error: no fmt chunk found\n");
        return {};
    }
    if (audioFormat != 1) {
        std::fprintf(stderr, "Error: only PCM format supported (got format %d)\n", audioFormat);
        return {};
    }
    if (bitsPerSample != 16) {
        std::fprintf(stderr, "Error: only 16-bit PCM supported (got %d bits)\n", bitsPerSample);
        return {};
    }
    if (dataSize == 0) {
        std::fprintf(stderr, "Error: no data chunk found\n");
        return {};
    }

    sampleRate  = static_cast<int>(sampleRateTemp);
    numChannels = numChannelsTemp;

    std::size_t totalSamples = dataSize / (bitsPerSample / 8);
    std::size_t frames = totalSamples / numChannels;
    std::vector<float> audio(frames, 0.0f);

    if (numChannels == 1) {
        for (std::size_t i = 0; i < frames; ++i) {
            int16_t sample;
            file.read(reinterpret_cast<char*>(&sample), 2);
            audio[i] = sample / 32768.0f;
        }
    } else {
        for (std::size_t i = 0; i < frames; ++i) {
            float sum = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch) {
                int16_t sample;
                file.read(reinterpret_cast<char*>(&sample), 2);
                sum += sample / 32768.0f;
            }
            audio[i] = sum / static_cast<float>(numChannels);
        }
    }

    return audio;
}

// ---------------------------------------------------------------------------
// BPM estimation via autocorrelation of onset detection function
// ---------------------------------------------------------------------------

float estimateBPM(const std::vector<float>& df, float sampleRate, std::size_t hopSize) {
    std::size_t len = df.size();
    if (len < 10) return 120.0f;

    float framesPerSec = sampleRate / static_cast<float>(hopSize);

    // Search lag range corresponding to 60–200 BPM
    int minLag = static_cast<int>(framesPerSec * 60.0f / 200.0f + 0.5f);
    int maxLag = static_cast<int>(framesPerSec * 60.0f / 60.0f + 0.5f);

    if (minLag < 4)  minLag = 4;
    if (maxLag > static_cast<int>(len / 2)) maxLag = static_cast<int>(len / 2);
    if (minLag >= maxLag) return 120.0f;

    // Normalize detection function
    float mean = 0.0f;
    for (float v : df) mean += v;
    mean /= static_cast<float>(len);

    std::vector<float> centered(len);
    float variance = 0.0f;
    for (std::size_t i = 0; i < len; ++i) {
        centered[i] = df[i] - mean;
        variance += centered[i] * centered[i];
    }
    if (variance < 1e-10f) return 120.0f;

    // Autocorrelation
    float bestCorr = -std::numeric_limits<float>::max();
    int bestLag = minLag;

    for (int lag = minLag; lag <= maxLag; ++lag) {
        float corr = 0.0f;
        for (std::size_t i = 0; i < len - static_cast<std::size_t>(lag); ++i) {
            corr += centered[i] * centered[i + static_cast<std::size_t>(lag)];
        }
        corr /= variance;

        if (corr > bestCorr) {
            bestCorr = corr;
            bestLag = lag;
        }
    }

    // Check for half-time / double-time by looking at sub-harmonics
    float bpm = framesPerSec * 60.0f / static_cast<float>(bestLag);

    // Refine: check if half the lag gives a higher correlation
    if (bestLag >= minLag * 2) {
        int halfLag = bestLag / 2;
        float halfCorr = 0.0f;
        for (std::size_t i = 0; i < len - static_cast<std::size_t>(halfLag); ++i) {
            halfCorr += centered[i] * centered[i + static_cast<std::size_t>(halfLag)];
        }
        halfCorr /= variance;
        if (halfCorr > bestCorr * 1.1f) {
            bpm = framesPerSec * 60.0f / static_cast<float>(halfLag);
        }
    }

    // Clamp to reasonable range
    if (bpm < 40.0f)  bpm *= 2.0f;
    if (bpm > 240.0f) bpm /= 2.0f;
    if (bpm < 40.0f)  bpm = 120.0f;
    if (bpm > 240.0f) bpm = 120.0f;

    return std::round(bpm * 10.0f) / 10.0f;
}

// ---------------------------------------------------------------------------
// Manual JSON writer (no external dependency)
// ---------------------------------------------------------------------------

void writeJsonString(std::FILE* out, const std::string& s) {
    std::fputc('"', out);
    for (char c : s) {
        switch (c) {
            case '"':  std::fprintf(out, "\\\""); break;
            case '\\': std::fprintf(out, "\\\\"); break;
            case '\n': std::fprintf(out, "\\n");  break;
            case '\r': std::fprintf(out, "\\r");  break;
            case '\t': std::fprintf(out, "\\t");  break;
            default:   std::fputc(c, out);        break;
        }
    }
    std::fputc('"', out);
}

void writeJsonArray(std::FILE* out, const std::vector<float>& values) {
    std::fputc('[', out);
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) std::fputc(',', out);
        std::fprintf(out, "%.3f", values[i]);
    }
    std::fputc(']', out);
}

void writeChordsJson(std::FILE* out,
                     const std::vector<aimidi::dsp::ChordResult>& chords) {
    std::fprintf(out, "  \"chords\": [\n");
    for (std::size_t i = 0; i < chords.size(); ++i) {
        if (i > 0) std::fprintf(out, ",\n");
        std::fprintf(out, "    {\"chord\": ");
        writeJsonString(out, chords[i].chord);
        std::fprintf(out, ", \"time_sec\": %.3f, \"confidence\": %.3f}",
                     chords[i].time_sec, chords[i].confidence);
    }
    std::fprintf(out, "\n  ],\n");
}

// ---------------------------------------------------------------------------
// CLI argument parsing
// ---------------------------------------------------------------------------

struct Args {
    std::string inputPath;
    std::string outputPath;
    bool        verbose = false;
};

void printUsage(const char* prog) {
    std::fprintf(stderr,
        "Usage: %s --input <file.wav> [--output <file.json>] [--verbose]\n"
        "\n"
        "Analyzes a WAV file and outputs audio features as JSON.\n"
        "Features: BPM, key, scale, chord progression, onsets.\n",
        prog);
}

bool parseArgs(int argc, char* argv[], Args& args) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--input") == 0) {
            if (++i >= argc) return false;
            args.inputPath = argv[i];
        } else if (std::strcmp(argv[i], "--output") == 0) {
            if (++i >= argc) return false;
            args.outputPath = argv[i];
        } else if (std::strcmp(argv[i], "--verbose") == 0) {
            args.verbose = true;
        } else if (std::strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            std::exit(0);
        } else {
            return false;
        }
    }
    return !args.inputPath.empty();
}

} // unnamed namespace

// ===========================================================================
int main(int argc, char* argv[]) {
    using namespace aimidi::dsp;

    Args args;
    if (!parseArgs(argc, argv, args)) {
        printUsage(argv[0]);
        return 1;
    }

    // ---- 1. Read WAV file ------------------------------------------------
    int sampleRate = 44100;
    int numChannels = 1;
    auto audio = readWAV(args.inputPath, sampleRate, numChannels);
    if (audio.empty()) {
        return 1;
    }

    float durationSec = static_cast<float>(audio.size()) / static_cast<float>(sampleRate);
    if (durationSec < 0.5f) {
        std::fprintf(stderr, "Error: audio too short (%.2f seconds)\n", durationSec);
        return 1;
    }

    if (args.verbose) {
        std::fprintf(stderr, "Loaded: %s (%d Hz, %d ch, %.2f sec, %zu samples)\n",
                     args.inputPath.c_str(), sampleRate, numChannels,
                     durationSec, audio.size());
    }

    // Analysis parameters
    const std::size_t fftSize  = 2048;
    const std::size_t hopSize  = 512;

    // ---- 2. Onset detection + BPM ----------------------------------------
    OnsetDetection onsetDetect(fftSize, hopSize);
    onsetDetect.process(audio.data(), audio.size(), static_cast<float>(sampleRate));

    auto onsets = onsetDetect.getOnsets(1.5f);
    const auto& df = onsetDetect.detectionFunction();

    float bpm = estimateBPM(df, static_cast<float>(sampleRate), hopSize);
    if (args.verbose) {
        std::fprintf(stderr, "Onsets: %zu events, estimated BPM: %.1f\n",
                     onsets.times.size(), bpm);
    }

    // ---- 3. Key detection ------------------------------------------------
    KeyDetection keyDetect(fftSize, hopSize, sampleRate);
    KeyResult keyResult = keyDetect.process(audio.data(), audio.size());

    if (args.verbose) {
        std::fprintf(stderr, "Key: %s %s (confidence: %.3f)\n",
                     keyResult.key.c_str(), keyResult.scale.c_str(),
                     keyResult.confidence);
    }

    // ---- 4. Chord detection ----------------------------------------------
    ChordDetection chordDetect(fftSize, hopSize, sampleRate);
    auto chords = chordDetect.process(audio.data(), audio.size());

    if (args.verbose) {
        std::fprintf(stderr, "Chords: %zu detected\n", chords.size());
        for (const auto& c : chords) {
            std::fprintf(stderr, "  %s @ %.2fs (conf: %.3f)\n",
                         c.chord.c_str(), c.time_sec, c.confidence);
        }
    }

    // ---- 5. Write JSON output --------------------------------------------
    std::FILE* out = stdout;
    bool closeFile = false;
    if (!args.outputPath.empty()) {
        out = std::fopen(args.outputPath.c_str(), "w");
        if (!out) {
            std::fprintf(stderr, "Error: cannot write '%s'\n",
                         args.outputPath.c_str());
            return 1;
        }
        closeFile = true;
    }

    std::fprintf(out, "{\n");
    std::fprintf(out, "  \"bpm\": %.1f,\n", bpm);
    std::fprintf(out, "  \"key\": ");
    writeJsonString(out, keyResult.key);
    std::fprintf(out, ",\n  \"scale\": ");
    writeJsonString(out, keyResult.scale);
    std::fprintf(out, ",\n  \"confidence\": %.4f,\n", keyResult.confidence);
    writeChordsJson(out, chords);
    std::fprintf(out, "  \"onsets\": ");
    writeJsonArray(out, onsets.times);
    std::fprintf(out, ",\n  \"duration_sec\": %.3f\n", durationSec);
    std::fprintf(out, "}\n");

    if (closeFile) {
        std::fclose(out);
        if (args.verbose) {
            std::fprintf(stderr, "Wrote analysis to '%s'\n",
                         args.outputPath.c_str());
        }
    }

    return 0;
}
