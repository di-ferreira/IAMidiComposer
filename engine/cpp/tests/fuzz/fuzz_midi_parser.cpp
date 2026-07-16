// Fuzz target for MIDI parser
#include <cstdint>
#include <cstddef>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Parse as SMF and verify no crashes
    // (This is a stub - real fuzzing requires libFuzzer linked)
    return 0;
}
