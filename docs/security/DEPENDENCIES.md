# Dependency Audit — AI MIDI Composer v0.1.0

## C++ Dependencies
| Dependency | Version | License | Purpose | Risk |
|---|---|---|---|---|
| JUCE | 7.0.12 | ISC | Plugin framework | Low |
| GoogleTest | 1.14.0 | BSD-3 | Testing | Low |
| spdlog | 1.x | MIT | Logging | Low |

## Go Dependencies
| Dependency | Version | License | Purpose | Risk |
|---|---|---|---|---|
| google.golang.org/grpc | 1.x | Apache-2.0 | gRPC | Low |
| modernc.org/sqlite | 1.x | BSD-3 | SQLite | Low |

## AI Dependencies
| Dependency | Version | License | Purpose | Risk |
|---|---|---|---|---|
| llama.cpp | latest | MIT | LLM inference | Low |
| ONNX Runtime | latest | MIT | ML inference | Low |

## Build Dependencies
| Dependency | Version | License | Purpose | Risk |
|---|---|---|---|---|
| CMake | 3.29+ | BSD-3 | Build system | Low |
| Go | 1.22+ | BSD-3 | Go compiler | Low |
| GCC/Clang | C++20 | GPL/LLVM | C++ compiler | Low |

## Security Notes
- All AI models run 100% offline
- No telemetry or network calls at runtime
- Plugin sandboxed by DAW host
- Input validation on all user prompts
- Fuzz testing on MIDI/proto parsers
