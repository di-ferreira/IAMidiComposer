# Threat Model — AI MIDI Composer v0.1.0

## Assets
1. User's musical compositions (MIDI data)
2. User's audio files (imported for W3)
3. User's plugin configuration/presets
4. AI model files (GGUF/ONNX)

## Threats

### T1: Malicious MIDI file
- **Risk**: Low
- **Mitigation**: Input validation on all MIDI imports, fuzz testing on parser
- **Status**: ✅ Fuzz target created

### T2: Malicious prompt
- **Risk**: Low
- **Mitigation**: InputValidation rejects literal note requests, length limits
- **Status**: ✅ Implemented in Sprint 10

### T3: Model file tampering
- **Risk**: Low (offline, user-downloaded)
- **Mitigation**: Checksum verification, sandboxed execution
- **Status**: ✅ Documented in model-safety skill

### T4: DAW sandbox escape
- **Risk**: Very Low (VST3 sandboxed by host)
- **Mitigation**: No file system access beyond plugin directory
- **Status**: ✅ Plugin uses JUCE's safe defaults

### T5: Data exfiltration
- **Risk**: None (100% offline)
- **Mitigation**: No network code in plugin or engine
- **Status**: ✅ Verified in architecture review
