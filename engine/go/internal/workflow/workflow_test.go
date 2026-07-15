package workflow

import (
	"bytes"
	"context"
	"encoding/binary"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"log/slog"
)

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

func newTestLogger() *slog.Logger {
	return slog.New(slog.NewTextHandler(&discardWriter{}, &slog.HandlerOptions{Level: slog.LevelDebug}))
}

type discardWriter struct{}

func (discardWriter) Write(p []byte) (int, error) { return len(p), nil }

// writeTestSMF writes a minimal valid SMF0 file with nTracks tracks.
func writeTestSMF(t *testing.T, path string, nTracks int) {
	t.Helper()
	f, err := os.Create(path)
	if err != nil {
		t.Fatalf("create test SMF: %v", err)
	}
	defer f.Close()

	hdr := struct {
		Magic    [4]byte
		HdrLen   uint32
		Format   uint16
		NTracks  uint16
		Division uint16
	}{[4]byte{'M', 'T', 'h', 'd'}, 6, 1, uint16(nTracks), 480}
	if err := binary.Write(f, binary.BigEndian, hdr); err != nil {
		t.Fatalf("write header: %v", err)
	}

	for range nTracks {
		magic := [4]byte{'M', 'T', 'r', 'k'}
		data := []byte{0x00, 0xFF, 0x2F, 0x00} // End of Track
		length := uint32(len(data))
		if err := binary.Write(f, binary.BigEndian, magic); err != nil {
			t.Fatalf("write track magic: %v", err)
		}
		if err := binary.Write(f, binary.BigEndian, length); err != nil {
			t.Fatalf("write track length: %v", err)
		}
		if _, err := f.Write(data); err != nil {
			t.Fatalf("write track data: %v", err)
		}
	}
}

// ---------------------------------------------------------------------------
// SMF helpers
// ---------------------------------------------------------------------------

func TestReadSMF_ValidFile(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "test.mid")
	writeTestSMF(t, path, 2)

	h, tracks, err := readSMF(path)
	if err != nil {
		t.Fatalf("readSMF: %v", err)
	}
	if string(h.Magic[:]) != "MThd" {
		t.Errorf("magic = %q, want MThd", h.Magic[:])
	}
	if h.NumTracks != 2 {
		t.Errorf("NumTracks = %d, want 2", h.NumTracks)
	}
	if len(tracks) != 2 {
		t.Errorf("len(tracks) = %d, want 2", len(tracks))
	}
}

func TestReadSMF_BadMagic(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "bad.mid")
	// Write a full 14-byte header with bogus magic.
	var buf bytes.Buffer
	binary.Write(&buf, binary.BigEndian, [4]byte{'B', 'O', 'G', 'U'})
	binary.Write(&buf, binary.BigEndian, uint32(6))
	binary.Write(&buf, binary.BigEndian, uint16(0))
	binary.Write(&buf, binary.BigEndian, uint16(0))
	binary.Write(&buf, binary.BigEndian, uint16(0))
	if err := os.WriteFile(path, buf.Bytes(), 0644); err != nil {
		t.Fatal(err)
	}
	_, _, err := readSMF(path)
	if err == nil || !strings.Contains(err.Error(), "bad magic") {
		t.Fatalf("expected bad magic error, got %v", err)
	}
}

func TestReadSMF_FileNotFound(t *testing.T) {
	_, _, err := readSMF("/nonexistent/mid/file.mid")
	if err == nil {
		t.Fatal("expected error for nonexistent file")
	}
}

func TestMergeSMF_AppendsTracks(t *testing.T) {
	dir := t.TempDir()

	basePath := filepath.Join(dir, "base.mid")
	overlayPath := filepath.Join(dir, "overlay.mid")
	mergedPath := filepath.Join(dir, "merged.mid")

	writeTestSMF(t, basePath, 2)
	writeTestSMF(t, overlayPath, 1)

	if err := mergeSMF(basePath, overlayPath, mergedPath); err != nil {
		t.Fatalf("mergeSMF: %v", err)
	}

	h, tracks, err := readSMF(mergedPath)
	if err != nil {
		t.Fatalf("read merged: %v", err)
	}
	if h.NumTracks != 3 {
		t.Errorf("merged NumTracks = %d, want 3", h.NumTracks)
	}
	if len(tracks) != 3 {
		t.Errorf("len(merged tracks) = %d, want 3", len(tracks))
	}
	if string(tracks[0].Magic[:]) != "MTrk" {
		t.Errorf("track 0 magic = %q", tracks[0].Magic[:])
	}
}

func TestMergeSMF_MissingBase(t *testing.T) {
	err := mergeSMF("/no/such/base.mid", "/no/such/over.mid", "/tmp/out.mid")
	if err == nil {
		t.Fatal("expected error for missing base file")
	}
}

// ---------------------------------------------------------------------------
// W6 — GenerateVariations (validation)
// ---------------------------------------------------------------------------

func TestGenerateVariations_InvalidCount(t *testing.T) {
	wm := New(newTestLogger(), nil)
	_, err := wm.GenerateVariations(context.Background(), &GenerateVariationsRequest{
		BaseSeed:       42,
		VariationCount: 0,
	})
	if err == nil || !strings.Contains(err.Error(), "variation_count") {
		t.Fatalf("expected variation_count error, got %v", err)
	}
}

func TestGenerateVariations_MissingMTE(t *testing.T) {
	// MTE_CLI won't be available in test, but the method should attempt
	// to call it and return an error (not panic).
	wm := New(newTestLogger(), nil)
	resp, err := wm.GenerateVariations(context.Background(), &GenerateVariationsRequest{
		BaseSeed:          42,
		VariationCount:    1,
		DeltaPerVariation: 10,
		BlueprintJSON:     `{"seed":42,"root_key":"C","scale":"major","bpm":120}`,
	})
	if err != nil {
		t.Fatalf("GenerateVariations returned error (expected partial): %v", err)
	}
	if resp == nil {
		t.Fatal("response is nil")
	}
	if len(resp.Variations) != 1 {
		t.Fatalf("len(variations) = %d, want 1", len(resp.Variations))
	}
	// The single variation should have failed (no mte_cli).
	if resp.Variations[0].SMFPath != "" {
		t.Errorf("expected empty SMFPath for failed variation, got %q", resp.Variations[0].SMFPath)
	}
	// But seed should still be set.
	if resp.Variations[0].Seed != 42 {
		t.Errorf("variation seed = %d, want 42", resp.Variations[0].Seed)
	}
}

// ---------------------------------------------------------------------------
// W2 — InstrumentComposer (validation)
// ---------------------------------------------------------------------------

func TestInstrumentComposer_EmptyInstrument(t *testing.T) {
	wm := New(newTestLogger(), nil)
	_, err := wm.InstrumentComposer(context.Background(), &InstrumentComposerRequest{
		Seed:       1,
		Instrument: "",
	})
	if err == nil || !strings.Contains(err.Error(), "instrument must not be empty") {
		t.Fatalf("expected empty instrument error, got %v", err)
	}
}

func TestInstrumentComposer_MissingMTE(t *testing.T) {
	wm := New(newTestLogger(), nil)
	_, err := wm.InstrumentComposer(context.Background(), &InstrumentComposerRequest{
		Seed:        1,
		Instrument:  "piano",
		BlueprintJSON: `{"seed":1,"root_key":"C","scale":"major","bpm":120}`,
	})
	if err == nil {
		t.Fatal("expected error (mte_cli not available)")
	}
	if !strings.Contains(err.Error(), "mte_cli") {
		t.Fatalf("error should mention mte_cli, got: %v", err)
	}
}

// ---------------------------------------------------------------------------
// W5 — SmartRegeneration (validation)
// ---------------------------------------------------------------------------

func TestSmartRegeneration_EmptyPath(t *testing.T) {
	wm := New(newTestLogger(), nil)
	_, err := wm.SmartRegeneration(context.Background(), &SmartRegenerationRequest{
		SMFPath: "",
	})
	if err == nil || !strings.Contains(err.Error(), "existing SMF path must not be empty") {
		t.Fatalf("expected empty path error, got %v", err)
	}
}

func TestSmartRegeneration_InvalidRegion(t *testing.T) {
	wm := New(newTestLogger(), nil)
	_, err := wm.SmartRegeneration(context.Background(), &SmartRegenerationRequest{
		SMFPath:     "/tmp/test.mid",
		RegionStart: 5,
		RegionEnd:   3,
	})
	if err == nil || !strings.Contains(err.Error(), "region_end") {
		t.Fatalf("expected region_end error, got %v", err)
	}
}

func TestSmartRegeneration_MissingSMF(t *testing.T) {
	wm := New(newTestLogger(), nil)
	_, err := wm.SmartRegeneration(context.Background(), &SmartRegenerationRequest{
		SMFPath:     "/nonexistent/file.mid",
		RegionStart: 0,
		RegionEnd:   4,
		Seed:        42,
		BlueprintJSON: `{"seed":42,"root_key":"C","scale":"major","bpm":120}`,
	})
	if err == nil {
		t.Fatal("expected error (no existing SMF)")
	}
	if !strings.Contains(err.Error(), "parse existing SMF") {
		t.Fatalf("error should mention parse, got: %v", err)
	}
}

// ---------------------------------------------------------------------------
// SMF merge end-to-end with real files
// ---------------------------------------------------------------------------

func TestSmartRegeneration_SMFAcceptsExistingFile(t *testing.T) {
	// Verify that a valid SMF is parsed correctly, and the workflow
	// proceeds to call mte_cli (which will fail, but the SMF parse
	// step should succeed).
	dir := t.TempDir()
	smfPath := filepath.Join(dir, "existing.mid")
	writeTestSMF(t, smfPath, 3)

	wm := New(newTestLogger(), nil)
	_, err := wm.SmartRegeneration(context.Background(), &SmartRegenerationRequest{
		SMFPath:     smfPath,
		RegionStart: 0,
		RegionEnd:   4,
		Seed:        42,
		BlueprintJSON: `{"seed":42,"root_key":"C","scale":"major","bpm":120}`,
	})
	if err == nil {
		t.Fatal("expected mte_cli error after SMF parse")
	}
	// Error should be about mte_cli / regeneration, not about SMF parsing.
	if strings.Contains(err.Error(), "parse existing SMF") {
		t.Fatalf("SMF parsing should have succeeded, got: %v", err)
	}
}

// ---------------------------------------------------------------------------
// Lock type serialization sanity
// ---------------------------------------------------------------------------

func TestLock_DefaultValues(t *testing.T) {
	l := Lock{}
	if l.Type != "" {
		t.Errorf("Lock.Type default = %q, want empty", l.Type)
	}
	if l.Instrument != "" {
		t.Errorf("Lock.Instrument default = %q, want empty", l.Instrument)
	}
}

// ---------------------------------------------------------------------------
// Edge cases: delta_per_variation == 0
// ---------------------------------------------------------------------------

func TestGenerateVariations_DefaultDelta(t *testing.T) {
	// When DeltaPerVariation is 0, the implementation should default to 1.
	wm := New(newTestLogger(), nil)
	resp, err := wm.GenerateVariations(context.Background(), &GenerateVariationsRequest{
		BaseSeed:          100,
		VariationCount:    2,
		DeltaPerVariation: 0,
		BlueprintJSON:     `{}`,
	})
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if len(resp.Variations) != 2 {
		t.Fatalf("len(variations) = %d, want 2", len(resp.Variations))
	}
	if resp.Variations[0].Seed != 100 {
		t.Errorf("variation 0 seed = %d, want 100", resp.Variations[0].Seed)
	}
	if resp.Variations[1].Seed != 101 {
		t.Errorf("variation 1 seed = %d, want 101", resp.Variations[1].Seed)
	}
}

// ---------------------------------------------------------------------------
// Golden: readSMF should reject files with bad header length
// ---------------------------------------------------------------------------

func TestReadSMF_BadHeaderLength(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "badhdr.mid")
	f, err := os.Create(path)
	if err != nil {
		t.Fatal(err)
	}
	// Write a full header with a bad hdrLen (99 instead of 6).
	binary.Write(f, binary.BigEndian, [4]byte{'M', 'T', 'h', 'd'})
	binary.Write(f, binary.BigEndian, uint32(99)) // bad length
	binary.Write(f, binary.BigEndian, uint16(0))
	binary.Write(f, binary.BigEndian, uint16(0))
	binary.Write(f, binary.BigEndian, uint16(0))
	f.Close()

	_, _, err = readSMF(path)
	if err == nil || !strings.Contains(err.Error(), "header length") {
		t.Fatalf("expected header length error, got %v", err)
	}
}

// ---------------------------------------------------------------------------
// Race-condition check: concurrent reads of SMF helpers
// ---------------------------------------------------------------------------

func TestReadSMF_Concurrent(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "conc.mid")
	writeTestSMF(t, path, 4)

	t.Run("parallel", func(t *testing.T) {
		t.Parallel()
		for range 10 {
			h, tracks, err := readSMF(path)
			if err != nil {
				t.Errorf("readSMF: %v", err)
			}
			if h.NumTracks != 4 || len(tracks) != 4 {
				t.Errorf("expected 4 tracks, got %d/%d", h.NumTracks, len(tracks))
			}
		}
	})
}

// ---------------------------------------------------------------------------
// InstrumentComposer with OutputPath copy
// ---------------------------------------------------------------------------

func TestInstrumentComposer_CopiesToOutputPath(t *testing.T) {
	// This tests that when OutputPath is set but mte_cli fails,
	// we get a proper error (not a panic).
	wm := New(newTestLogger(), nil)
	_, err := wm.InstrumentComposer(context.Background(), &InstrumentComposerRequest{
		Seed:        99,
		Instrument:  "bass",
		OutputPath:  filepath.Join(t.TempDir(), "bass.mid"),
		BlueprintJSON: `{"seed":99,"root_key":"D","scale":"minor","bpm":100}`,
	})
	if err == nil {
		t.Fatal("expected error (mte_cli not available)")
	}
	if strings.Contains(err.Error(), "copy SMF") {
		t.Fatal("should not reach copy stage if mte_cli fails")
	}
}

// ---------------------------------------------------------------------------
// Memory: verify large SMF doesn't blow up
// ---------------------------------------------------------------------------

func TestReadSMF_LargeTrack(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "large.mid")
	f, err := os.Create(path)
	if err != nil {
		t.Fatal(err)
	}

	hdr := struct {
		Magic    [4]byte
		HdrLen   uint32
		Format   uint16
		NTracks  uint16
		Division uint16
	}{[4]byte{'M', 'T', 'h', 'd'}, 6, 1, 1, 480}
	if err := binary.Write(f, binary.BigEndian, hdr); err != nil {
		t.Fatal(err)
	}

	trackData := bytes.Repeat([]byte{0x90, 0x40, 0x40, 0x00}, 1024)
	magic := [4]byte{'M', 'T', 'r', 'k'}
	length := uint32(len(trackData))
	if err := binary.Write(f, binary.BigEndian, magic); err != nil {
		t.Fatal(err)
	}
	if err := binary.Write(f, binary.BigEndian, length); err != nil {
		t.Fatal(err)
	}
	if _, err := f.Write(trackData); err != nil {
		t.Fatal(err)
	}
	f.Close()

	h, tracks, err := readSMF(path)
	if err != nil {
		t.Fatalf("readSMF large track: %v", err)
	}
	if h.NumTracks != 1 {
		t.Errorf("NumTracks = %d, want 1", h.NumTracks)
	}
	if len(tracks) != 1 {
		t.Errorf("len(tracks) = %d, want 1", len(tracks))
	}
	if len(tracks[0].Data) != len(trackData) {
		t.Errorf("track data length = %d, want %d", len(tracks[0].Data), len(trackData))
	}
}
