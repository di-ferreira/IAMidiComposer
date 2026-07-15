package workflow

import (
	"context"
	"encoding/binary"
	"fmt"
	"log/slog"
	"math/rand"
	"os"
	"os/exec"
	"path/filepath"

	"aimidi.composer/ace/internal/store"
)

// WorkflowManager orchestrates all composition workflows,
// delegating to the mte_cli subprocess for actual MIDI generation.
//
// Thread-safety: all exported methods are safe for concurrent use
// as long as each context carries its own cancellation; the manager
// itself holds no mutable shared state.
type WorkflowManager struct {
	mteCliPath string
	tmpDir     string
	logger     *slog.Logger
	ctxRepo    *store.ContextRepository
}

// New creates a WorkflowManager. The store may be nil if Shared Musical
// Context persistence is not required (e.g. in tests).
func New(logger *slog.Logger, s *store.Store) *WorkflowManager {
	mtePath := os.Getenv("MTE_CLI_PATH")
	if mtePath == "" {
		mtePath = "mte_cli"
	}
	tmpDir := os.Getenv("AIMIDI_TMPDIR")
	if tmpDir == "" {
		var err error
		tmpDir, err = os.MkdirTemp("", "aimidi-mte-*")
		if err != nil {
			logger.Warn("failed to create temp dir, falling back to /tmp", "error", err)
			tmpDir = "/tmp"
		}
	}
	var ctxRepo *store.ContextRepository
	if s != nil {
		ctxRepo = store.NewContextRepository(s)
	}
	return &WorkflowManager{
		mteCliPath: mtePath,
		tmpDir:     tmpDir,
		logger:     logger,
		ctxRepo:    ctxRepo,
	}
}

// runMTE executes mte_cli with the given extra args and returns the path
// to the generated SMF file. The seed is always passed.
func (wm *WorkflowManager) runMTE(ctx context.Context, extraArgs []string, seed int64) (string, error) {
	outputFile := filepath.Join(wm.tmpDir, fmt.Sprintf("out_%d_%x.mid", seed, rand.Uint64()))

	args := make([]string, 0, len(extraArgs)+4)
	args = append(args,
		"--seed", fmt.Sprintf("%d", seed),
		"--output", outputFile,
	)
	args = append(args, extraArgs...)

	cmd := exec.CommandContext(ctx, wm.mteCliPath, args...)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("mte_cli failed: %w\noutput: %s", err, string(output))
	}

	if _, statErr := os.Stat(outputFile); statErr != nil {
		return "", fmt.Errorf("mte_cli exited ok but output file %q not found: %w", outputFile, statErr)
	}

	return outputFile, nil
}

// ---------------------------------------------------------------------------
// SMF helpers (used by SmartRegeneration)
// ---------------------------------------------------------------------------

// smfHeader is the fixed-size SMF header (14 bytes).
type smfHeader struct {
	Magic    [4]byte // "MThd"
	HdrLen   uint32  // always 6
	Format   uint16  // 0 or 1
	NumTracks uint16
	Division uint16
}

// smfTrack is a single SMF track chunk.
type smfTrack struct {
	Magic  [4]byte // "MTrk"
	Length uint32
	Data   []byte
}

// readSMF parses an SMF file into its header and track slices.
func readSMF(path string) (*smfHeader, []smfTrack, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, nil, err
	}
	defer f.Close()

	var h smfHeader
	if err := binary.Read(f, binary.BigEndian, &h); err != nil {
		return nil, nil, fmt.Errorf("reading SMF header: %w", err)
	}
	if string(h.Magic[:]) != "MThd" {
		return nil, nil, fmt.Errorf("not an SMF file: bad magic %q", h.Magic[:])
	}
	if h.HdrLen != 6 {
		return nil, nil, fmt.Errorf("unexpected header length %d", h.HdrLen)
	}

	tracks := make([]smfTrack, h.NumTracks)
	for i := range tracks {
		if err := binary.Read(f, binary.BigEndian, &tracks[i].Magic); err != nil {
			return nil, nil, fmt.Errorf("reading track %d magic: %w", i, err)
		}
		if string(tracks[i].Magic[:]) != "MTrk" {
			return nil, nil, fmt.Errorf("track %d: bad magic %q", i, tracks[i].Magic[:])
		}
		if err := binary.Read(f, binary.BigEndian, &tracks[i].Length); err != nil {
			return nil, nil, fmt.Errorf("reading track %d length: %w", i, err)
		}
		tracks[i].Data = make([]byte, tracks[i].Length)
		if _, err := f.Read(tracks[i].Data); err != nil {
			return nil, nil, fmt.Errorf("reading track %d data: %w", i, err)
		}
	}
	return &h, tracks, nil
}

// mergeSMF concatenates the tracks from base and overlay into a single
// SMF file, preserving the header from base but updating NumTracks.
// Tracks from overlay are appended after all base tracks.
func mergeSMF(basePath, overlayPath, outputPath string) error {
	baseH, baseTracks, err := readSMF(basePath)
	if err != nil {
		return fmt.Errorf("read base SMF: %w", err)
	}
	_, overlayTracks, err := readSMF(overlayPath)
	if err != nil {
		return fmt.Errorf("read overlay SMF: %w", err)
	}

	merged := baseTracks
	merged = append(merged, overlayTracks...)
	baseH.NumTracks = uint16(len(merged))

	out, err := os.Create(outputPath)
	if err != nil {
		return fmt.Errorf("create merged SMF: %w", err)
	}
	defer out.Close()

	if err := binary.Write(out, binary.BigEndian, baseH); err != nil {
		return fmt.Errorf("write merged header: %w", err)
	}
	for i, t := range merged {
		t.Length = uint32(len(t.Data))
		if err := binary.Write(out, binary.BigEndian, &t.Magic); err != nil {
			return fmt.Errorf("write track %d magic: %w", i, err)
		}
		if err := binary.Write(out, binary.BigEndian, t.Length); err != nil {
			return fmt.Errorf("write track %d length: %w", i, err)
		}
		if _, err := out.Write(t.Data); err != nil {
			return fmt.Errorf("write track %d data: %w", i, err)
		}
	}
	return nil
}
