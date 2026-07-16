package workflow

import (
	"context"
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// ---------------------------------------------------------------------------
// W3 — AudioAssisted (validation)
// ---------------------------------------------------------------------------

func TestAudioAssisted_EmptyPath(t *testing.T) {
	wm := New(newTestLogger(), nil)
	_, err := wm.AudioAssisted(context.Background(), &AudioAssistedRequest{
		AudioPath: "",
	})
	if err == nil || !strings.Contains(err.Error(), "audio path must not be empty") {
		t.Fatalf("expected empty path error, got %v", err)
	}
}

func TestAudioAssisted_InvalidPath(t *testing.T) {
	wm := New(newTestLogger(), nil)
	_, err := wm.AudioAssisted(context.Background(), &AudioAssistedRequest{
		AudioPath: "/nonexistent/file.wav",
	})
	if err == nil {
		t.Fatal("expected error for invalid audio path")
	}
	if !strings.Contains(err.Error(), "audio file not accessible") {
		t.Fatalf("expected 'audio file not accessible' error, got %v", err)
	}
}

func TestAudioAssisted_MissingDSPCli(t *testing.T) {
	wm := New(newTestLogger(), nil)
	// Use a real existing file so the stat check passes, but dsp_cli won't be found.
	// We create a minimal valid WAV file to get past the accessibility check.
	dir := t.TempDir()
	wavPath := filepath.Join(dir, "test.wav")
	writeMinimalWAV(t, wavPath)

	_, err := wm.AudioAssisted(context.Background(), &AudioAssistedRequest{
		AudioPath: wavPath,
		Seed:      42,
	})
	if err == nil {
		t.Fatal("expected error (dsp_cli not available)")
	}
	if !strings.Contains(err.Error(), "dsp_cli") {
		t.Fatalf("error should mention dsp_cli, got: %v", err)
	}
}

func TestAudioAssisted_InvalidWAV(t *testing.T) {
	wm := New(newTestLogger(), nil)
	dir := t.TempDir()
	badPath := filepath.Join(dir, "not_wav.bin")
	if err := os.WriteFile(badPath, []byte("this is not a wav file"), 0644); err != nil {
		t.Fatal(err)
	}

	_, err := wm.AudioAssisted(context.Background(), &AudioAssistedRequest{
		AudioPath: badPath,
		Seed:      42,
	})
	if err == nil {
		t.Fatal("expected error (dsp_cli should reject invalid WAV)")
	}
}

// ---------------------------------------------------------------------------
// AudioAnalysisResult JSON round-trip
// ---------------------------------------------------------------------------

func TestAudioAnalysisResult_JSONRoundTrip(t *testing.T) {
	original := AudioAnalysisResult{
		BPM:        120.5,
		Key:        "C",
		Scale:      "major",
		Confidence: 0.85,
		Chords: []DetectedChord{
			{Chord: "C", TimeSec: 0.0, Confidence: 0.9},
			{Chord: "G", TimeSec: 2.0, Confidence: 0.85},
			{Chord: "Am", TimeSec: 4.0, Confidence: 0.8},
		},
		Onsets:      []float64{0.0, 0.25, 0.5, 0.75, 1.0},
		DurationSec: 8.0,
	}

	data, err := json.Marshal(original)
	if err != nil {
		t.Fatalf("marshal: %v", err)
	}

	var decoded AudioAnalysisResult
	if err := json.Unmarshal(data, &decoded); err != nil {
		t.Fatalf("unmarshal: %v", err)
	}

	if decoded.BPM != original.BPM {
		t.Errorf("BPM = %f, want %f", decoded.BPM, original.BPM)
	}
	if decoded.Key != original.Key {
		t.Errorf("Key = %q, want %q", decoded.Key, original.Key)
	}
	if decoded.Scale != original.Scale {
		t.Errorf("Scale = %q, want %q", decoded.Scale, original.Scale)
	}
	if decoded.Confidence != original.Confidence {
		t.Errorf("Confidence = %f, want %f", decoded.Confidence, original.Confidence)
	}
	if len(decoded.Chords) != len(original.Chords) {
		t.Errorf("len(chords) = %d, want %d", len(decoded.Chords), len(original.Chords))
	}
	if len(decoded.Onsets) != len(original.Onsets) {
		t.Errorf("len(onsets) = %d, want %d", len(decoded.Onsets), len(original.Onsets))
	}
	if decoded.DurationSec != original.DurationSec {
		t.Errorf("DurationSec = %f, want %f", decoded.DurationSec, original.DurationSec)
	}
}

func TestAudioAnalysisResult_EmptyOnsets(t *testing.T) {
	result := AudioAnalysisResult{
		BPM:    100,
		Key:    "D",
		Scale:  "minor",
		Onsets: []float64{},
	}

	data, err := json.Marshal(result)
	if err != nil {
		t.Fatalf("marshal: %v", err)
	}

	var decoded AudioAnalysisResult
	if err := json.Unmarshal(data, &decoded); err != nil {
		t.Fatalf("unmarshal: %v", err)
	}

	if len(decoded.Onsets) != 0 {
		t.Errorf("len(onsets) = %d, want 0", len(decoded.Onsets))
	}
}

func TestDetectedChord_JSONTags(t *testing.T) {
	dc := DetectedChord{Chord: "G7", TimeSec: 1.5, Confidence: 0.75}
	data, err := json.Marshal(dc)
	if err != nil {
		t.Fatalf("marshal: %v", err)
	}

	var decoded DetectedChord
	if err := json.Unmarshal(data, &decoded); err != nil {
		t.Fatalf("unmarshal: %v", err)
	}

	if decoded.Chord != "G7" {
		t.Errorf("Chord = %q, want G7", decoded.Chord)
	}
	if decoded.TimeSec != 1.5 {
		t.Errorf("TimeSec = %f, want 1.5", decoded.TimeSec)
	}
	if decoded.Confidence != 0.75 {
		t.Errorf("Confidence = %f, want 0.75", decoded.Confidence)
	}
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// writeMinimalWAV writes the smallest valid PCM 16-bit mono WAV file
// containing 1 second of silence at 44100 Hz.
func writeMinimalWAV(t *testing.T, path string) {
	t.Helper()

	f, err := os.Create(path)
	if err != nil {
		t.Fatalf("create WAV: %v", err)
	}
	defer f.Close()

	sampleRate := uint32(44100)
	numChannels := uint16(1)
	bitsPerSample := uint16(16)
	numSamples := sampleRate // 1 second
	dataSize := numSamples * uint32(bitsPerSample/8)

	// WAV header
	hdr := make([]byte, 44)
	copy(hdr[0:4], []byte("RIFF"))
	putLE32(hdr[4:8], 36+dataSize) // file size - 8
	copy(hdr[8:12], []byte("WAVE"))
	copy(hdr[12:16], []byte("fmt "))
	putLE32(hdr[16:20], 16)                      // fmt chunk size
	putLE16(hdr[20:22], 1)                       // PCM
	putLE16(hdr[22:24], numChannels)
	putLE32(hdr[24:28], sampleRate)
	putLE32(hdr[28:32], sampleRate*uint32(numChannels)*uint32(bitsPerSample/8))
	putLE16(hdr[32:34], uint16(numChannels)*uint16(bitsPerSample/8))
	putLE16(hdr[34:36], bitsPerSample)
	copy(hdr[36:40], []byte("data"))
	putLE32(hdr[40:44], dataSize)

	if _, err := f.Write(hdr); err != nil {
		t.Fatalf("write header: %v", err)
	}

	// Write silent samples
	silence := make([]byte, dataSize)
	if _, err := f.Write(silence); err != nil {
		t.Fatalf("write samples: %v", err)
	}
}

func putLE16(buf []byte, v uint16) {
	buf[0] = byte(v)
	buf[1] = byte(v >> 8)
}

func putLE32(buf []byte, v uint32) {
	buf[0] = byte(v)
	buf[1] = byte(v >> 8)
	buf[2] = byte(v >> 16)
	buf[3] = byte(v >> 24)
}
