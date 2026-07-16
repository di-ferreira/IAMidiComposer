# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2026-07-16

### Added

#### MTE Core
- **HarmonyEngine** — progressions, substitutions, roman numeral resolution, modulation
- **ChordEngine** — voicings (close, open, drop-2, spread), voice leading, register-aware placement
- **BassEngine** — walking lines, root-fifth patterns, pedal points, syncopated grooves
- **DrumEngine** — grooves for rock, funk, jazz, pop, electronic; ghost notes, fills, transitions
- **PianoEngine** — two-hand voicing, arpeggios, comping, lead lines, sustain pedal
- **GuitarEngine** — strumming, palm muting, arpeggios, clean/distorted patterns
- **StringsEngine** — section voicing (violin I/II, viola, cello, contrabass), legato, dynamics

#### MTE Advanced
- **ModulationEngine** — pivot chord, direct modulation, common-tone modulation
- **CounterpointEngine** — species 1–5, dissonance treatment, independent voice leading
- **OrchestrationEngine** — instrument ranges, doublings, texture balancing, tutti/tutti partitioning
- **ReharmonizeEngine** — chord substitution (diatonic, tritone, borrowed), re-voicing

#### Musical Intelligence
- **PromptInterpreter** — natural language prompt → structured musical intent
- **StyleDetector** — genre/mood classification from prompt or audio features
- **BlueprintGenerator** — compiles intents into reproducible Music Blueprint
- **TimelinePlanner** — section layout, bar counts, transitions
- **ArrangementPlanner** — instrument entry/exit, dynamic contrast, texture density
- **InstrumentMapper** — maps arranjo instruments to available VST3/VSTi programs

#### DSP Engine
- **FFT** — radix-2, configurable windowing, overlap-add STFT
- **Windowing** — Hann, Hamming, Blackman, Blackman-Harris, Kaiser
- **Onset Detection** — spectral flux, HFC, complex domain
- **Tempo Tracking** — auto-correlation, dynamic BPM estimation
- **Key Detection** — Krumhansl-Schmuckler, alternative candidate ranking
- **Chord Detection** — chromagram-based chord labeling with timeline

#### Workflows
- **W1: New Composition** — full composition from natural language prompt
- **W2: Instrument Composer** — single-instrument generation over existing context
- **W3: Audio Assisted Composer** — analyze audio, create complementary MIDI
- **W4: Continue Composition** — extend existing composition from cursor
- **W5: Smart Regeneration** — regenerate bounded region preserving context
- **W6: Generate Variations** — N variations of existing composition (same seed family)
- **W7: Replace Instrument** — adapt MIDI to different instrument/timbre
- **W8: Reharmonize** — new harmony preserving melody structure
- **W9: Orchestrate** — expand simple idea into full orchestral arrangement
- **W10: Arrange** — auto-accompaniment with style-appropriate backing

#### Plugin UI
- Dark theme with adjustable brightness/contrast
- Prompt input box with history (last 50 prompts)
- Workflow selector with per-workflow configuration panel
- Piano roll preview with zoom, scroll, note selection
- Region selection for targeted regeneration
- Drag-and-drop SMF export to DAW timeline
- Progress indicator for long-running compositions
- Settings panel: model path, seed override, output preferences

#### Plugin Features
- APVTS automation parameters for DAW automation of key controls
- PresetManager with 5 factory presets (Pop, Jazz, Rock, Orchestral, Electronic)
- PluginScanner — enumerates VST3/VSTi on host
- State serialization via ValueTree (save/load project state)

#### Infrastructure
- gRPC server (MTE C++ ↔ ACE Go) over Unix domain socket / named pipe
- SQLite-backed Shared Musical Context (tempo, key, scale, progression, history)
- SPSC lock-free FIFO for audio-thread-safe MIDI event delivery
- ServiceLocator DI container for C++ MTE modules
- CLI toolchain: `mte_cli`, `prompt_interpreter`, `midi_renderer`

#### Hardening
- Fuzz targets for MIDI parser, proto deserialization, blueprint validator
- Dependency audit automation (Dependabot, govulncheck, clang-tidy)
- Threat model documented in `docs/security/threat_model.md`
- 320+ unit/integration tests across C++ (GoogleTest) and Go (table-driven)
- Coverage analysis with gcovr + go test -cover

#### Benchmarks & Profiling
- `docs/benchmarks/` — latency, throughput, memory benchmarks for MTE hot paths
- Tracy profiling hooks in all MTE engines
- Go pprof endpoints for ACE server profiling

### Technical
- C++20 with concepts, ranges, constexpr, consteval, constinit
- Go 1.22+ with context.Context, gRPC, modernc.org/sqlite (pure Go)
- JUCE 7.0.12 VST3 plugin framework
- 100% offline after initial model download
- Seed-deterministic: same seed + same prompt = identical MIDI output
- Cross-platform: Linux (Ubuntu 22.04+), macOS (13+), Windows (2022+)

[0.1.0]: https://github.com/di-ferreira/IAMidiComposer/releases/tag/v0.1.0
