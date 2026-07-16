# AI MIDI Composer

[![CI](https://github.com/di-ferreira/IAMidiComposer/actions/workflows/ci.yml/badge.svg)](https://github.com/di-ferreira/IAMidiComposer/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-0.1.0--alpha-green)](https://github.com/di-ferreira/IAMidiComposer/releases/tag/v0.1.0)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C)](https://en.cppreference.com/w/cpp/20)
[![Go](https://img.shields.io/badge/Go-1.22+-00ADD8)](https://go.dev/)
[![JUCE](https://img.shields.io/badge/JUCE-7.0.12-8B5CFE)](https://juce.com/)

**AI MIDI Composer** is an open-source, 100% offline AI-assisted music composition platform. The AI interprets your musical intent; a deterministic Music Theory Engine generates seed-reproducible MIDI. You keep full creative control in your DAW.

---

## Quick Start

```bash
# Build from source
git clone https://github.com/di-ferreira/IAMidiComposer.git
cd IAMidiComposer
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Download AI models (first run only)
./scripts/install_models.sh

# Generate your first composition via CLI
./build/bin/mte_cli --prompt "upbeat pop in C major, 8 bars" --seed 42 --output my_song.mid
```

Full guide: [`QUICKSTART.md`](QUICKSTART.md)

---

## Features

- **10 Composition Workflows** — New Composition, Instrument Composer, Audio Assisted Composer, Continue Composition, Smart Regeneration, Generate Variations, Replace Instrument, Reharmonize, Orchestrate, Arrange
- **Music Theory Engine** — Harmony, Chords, Voice Leading, Counterpoint, Melody, Rhythm, Bass, Drums, Guitar, Piano, Strings, Orchestration, Humanization
- **Musical Intelligence** — Natural language prompt interpretation, style/mood detection, blueprint generation, timeline planning, arrangement planning
- **DSP Engine** — FFT, onset detection, tempo tracking, key detection, chord detection
- **VST3 Plugin** — Dark theme, piano roll preview, prompt history, workflow selector, drag-and-drop SMF export
- **100% Offline** — Local AI (llama.cpp GGUF + ONNX Runtime), no cloud dependencies
- **Seed-Deterministic** — Same seed + same prompt = identical MIDI output
- **Cross-Platform** — Linux, macOS, Windows
- **CLI & Plugin** — Use from terminal or inside your DAW

---

## Architecture

```
┌────────────────────────────────────────────────────────────────────┐
│                          DAW (VST3 Host)                           │
│     Cubase · Reaper · Studio One · FL Studio · Ableton · Bitwig    │
└────────────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌────────────────────────────────────────────────────────────────────┐
│                   AI MIDI Plugin (JUCE / C++)                      │
│  Prompt UI · Piano Roll · Workflow Selector · Region Editor        │
│  Preset Manager · Plugin Scanner · Settings                        │
└───────────┬────────────────────────────────────────────────────────┘
            │ gRPC (Unix socket / named pipe)
            ▼
╔════════════════════════════════════════════════════════════════════╗
║              AI Composition Engine (ACE) — Go                     ║
╠════════════════════════════════════════════════════════════════════╣
║  Shared Musical Context (SQLite) · Musical Intelligence            ║
║  Workflow Manager · Prompt Interpreter · Style Detector            ║
║  Blueprint Generator · Audio Analysis Layer                        ║
╚════════════════════════════════════════════════════════════════════╝
            │ gRPC (Unix socket / named pipe)
            ▼
╔════════════════════════════════════════════════════════════════════╗
║            Music Theory Engine (MTE) — C++20                      ║
╠════════════════════════════════════════════════════════════════════╣
║  Harmony · Chords · Bass · Drums · Piano · Guitar · Strings       ║
║  Counterpoint · Orchestration · Modulation · Humanization          ║
║  MIDI Rendering Engine → SMF / event stream                       ║
╚════════════════════════════════════════════════════════════════════╝
```

---

## Documentation

| Resource | Description |
|---|---|
| [`QUICKSTART.md`](QUICKSTART.md) | 5-minute setup guide |
| [`CONTEXT.md`](CONTEXT.md) | Mission, philosophy, scope, objectives |
| [`CHANGELOG.md`](CHANGELOG.md) | Release history |
| [`docs/adr/`](docs/adr/) | Architecture Decision Records |
| [`docs/roadmap/ROADMAP.md`](docs/roadmap/ROADMAP.md) | Full roadmap M0–M7 |
| [`docs/benchmarks/`](docs/benchmarks/) | Performance benchmarks |
| [`docs/workflows/`](docs/workflows/) | Workflow documentation |
| [`standards/`](standards/) | Architecture & code standards |
| [`proto/aimidi/v1/`](proto/aimidi/v1/) | Protocol Buffers schema |

---

## Stack

| Layer | Technology |
|---|---|
| Plugin | C++20, JUCE 7.0.12 |
| Music Engine | C++20 (concepts, ranges, constexpr) |
| AI Engine | Go 1.22+, gRPC, modernc.org/sqlite |
| Local AI | llama.cpp (GGUF), ONNX Runtime |
| Build | CMake 3.29+, Ninja |
| Tests | GoogleTest, Go table-driven tests |
| CI/CD | GitHub Actions (Linux, macOS, Windows) |

---

## License

Apache 2.0. See [LICENSE](LICENSE).

---

## Contributing

See [`standards/branching.md`](standards/branching.md), [`standards/pull_request.md`](standards/pull_request.md), and [`standards/commits.md`](standards/commits.md) for contribution guidelines.
