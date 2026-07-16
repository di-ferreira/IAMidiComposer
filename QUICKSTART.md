# Quickstart — AI MIDI Composer v0.1.0

## 5 minutes to your first composition

### Prerequisites

- **OS**: Linux (Ubuntu 22.04+), macOS (13+), or Windows (2022+)
- **DAW**: Any VST3-compatible DAW (Cubase, Reaper, Studio One, FL Studio, Ableton Live, Bitwig)
- **Tools**: CMake 3.29+, C++20 compiler (clang 16+ / GCC 13+), Go 1.22+
- **Disk**: ~3 GB free (2 GB for AI model, ~500 MB for build artifacts)
- **RAM**: 8 GB minimum (16 GB recommended for LLM inference)

### Build

```bash
git clone https://github.com/di-ferreira/IAMidiComposer.git
cd IAMidiComposer

# Configure
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build all targets
cmake --build build -j$(nproc)

# (Optional) Build and run tests
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DAIMIDI_BUILD_TESTS=ON
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

### Install AI Models

```bash
# Automatic download (requires internet)
./scripts/install_models.sh

# Offline install (pre-downloaded models)
./scripts/install_models.sh --offline --source /path/to/models/
```

The script downloads the default model (Llama-3.2-3B-Instruct Q4_K_M, ~2 GB) to `~/.local/share/ai-midi-composer/models/`.

**No model? No problem.** The system falls back to a rule-based intent parser — limited but functional.

### Install Plugin

Copy the VST3 bundle to your DAW's VST3 folder:

| Platform | Path |
|---|---|
| Linux | `~/.vst3/` |
| macOS | `~/Library/Audio/Plug-Ins/VST3/` |
| Windows | `C:\Program Files\Common Files\VST3\` |

```bash
# Linux example
cp -r build/plugin/ai_midi_composer_artefacts/Release/VST3/AI\ MIDI\ Composer.vst3 ~/.vst3/
```

### Use

#### In your DAW
1. Load the plugin on a MIDI track
2. Type a prompt (e.g. "warm jazz piano ballad in C minor")
3. Select a workflow from the dropdown
4. Click **Generate**
5. Drag the MIDI from the piano roll to your DAW's timeline

#### Via CLI (no DAW required)
```bash
# Full composition
./build/bin/mte_cli --prompt "upbeat pop in C major, 8 bars" --seed 42 --output my_song.mid

# Instrument-specific
./build/bin/mte_cli --workflow instrument --instrument drums --prompt "funk groove 4/4" --seed 7

# Listen to the result with any MIDI player
# (e.g. timidity, fluidsynth, or import into your DAW)
```

### Example Prompts

| Prompt | Style | Output |
|---|---|---|
| `"upbeat pop in C major, 8 bars"` | Pop | Full band arrangement |
| `"warm jazz piano ballad in C minor"` | Jazz | Solo piano with voicings |
| `"heavy rock riff in E"` | Rock | Distorted guitar + drums |
| `"cinematic strings in D minor, slow"` | Orchestral | String section arrangement |
| `"deep house groove, 128 BPM"` | Electronic | Drums + bass + pads |

### Next Steps

- Read the [Architecture Decision Records](docs/adr/) to understand design choices
- Explore [workflow documentation](docs/workflows/) for detailed workflow usage
- Check [benchmarks](docs/benchmarks/) for performance characteristics
- Browse the [standards](standards/) for contribution guidelines

### Troubleshooting

| Problem | Solution |
|---|---|
| `cmake` not found | Install CMake 3.29+: `apt install cmake` / `brew install cmake` / `choco install cmake` |
| `clang++` not found | Install clang 16+: `apt install clang` / `brew install llvm` / `choco install llvm` |
| Plugin not visible in DAW | Verify VST3 path; rescan plugins in DAW settings |
| MIDI output is empty | Check that models are installed (`./scripts/install_models.sh --check`) |
| High CPU usage | AI inference runs on CPU; expect 100% on one core during generation |
| "Model not found" error | Run `install_models.sh` or set `AIMIDI_MODEL_PATH` to a valid GGUF file |
