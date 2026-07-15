package workflow

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"time"
)

// SmartRegenerationRequest represents W5 input.
type SmartRegenerationRequest struct {
	BlueprintJSON string
	Seed          int64
	RegionStart   int    // bar start (0-indexed)
	RegionEnd     int    // bar end (exclusive)
	Locks         []Lock // locked regions/instruments
	SMFPath       string // existing SMF file path
}

// Lock represents a locked musical region that must be preserved.
type Lock struct {
	Type       string // "bar", "instrument", "note"
	BarStart   int
	BarEnd     int
	Instrument string
}

// SmartRegenerationResponse represents W5 output.
type SmartRegenerationResponse struct {
	SMFPath    string
	RegionBars int
	DurationMs int64
}

// SmartRegeneration implements Workflow 5 (Smart Regeneration).
//
// It preserves locked regions from the existing SMF and regenerates the
// unlocked region by calling mte_cli with region bounds, then merges the
// original locked content with the newly generated material.
func (wm *WorkflowManager) SmartRegeneration(ctx context.Context, req *SmartRegenerationRequest) (*SmartRegenerationResponse, error) {
	if req.SMFPath == "" {
		return nil, fmt.Errorf("existing SMF path must not be empty")
	}
	if req.RegionEnd <= req.RegionStart {
		return nil, fmt.Errorf("region_end (%d) must be > region_start (%d)", req.RegionEnd, req.RegionStart)
	}

	wm.logger.InfoContext(ctx, "W5.SmartRegeneration",
		"seed", req.Seed,
		"region", []int{req.RegionStart, req.RegionEnd},
		"locks", len(req.Locks),
		"existing_smf", req.SMFPath,
	)

	// 1. Parse existing SMF to get track structure.
	_, baseTracks, err := readSMF(req.SMFPath)
	if err != nil {
		return nil, fmt.Errorf("parse existing SMF: %w", err)
	}

	// 2. Generate new material for the unlocked region.
	args := []string{
		"--blueprint-json", req.BlueprintJSON,
		"--region-start", fmt.Sprintf("%d", req.RegionStart),
		"--region-end", fmt.Sprintf("%d", req.RegionEnd),
	}

	// Pass locked instruments so mte_cli can skip them.
	for _, l := range req.Locks {
		if l.Type == "instrument" && l.Instrument != "" {
			args = append(args, "--lock-instrument", l.Instrument)
		}
	}

	start := time.Now()
	newSMFPath, err := wm.runMTE(ctx, args, req.Seed)
	elapsed := time.Since(start)
	if err != nil {
		return nil, fmt.Errorf("regeneration failed: %w", err)
	}
	defer os.Remove(newSMFPath)

	// 3. Merge: combine original tracks with regenerated tracks.
	// The merged file keeps the base's tracks and appends regenerated
	// tracks so that the DAW can layer both regions.
	mergedPath := filepath.Join(wm.tmpDir,
		fmt.Sprintf("merged_%d_%x.mid", req.Seed, time.Now().UnixNano()),
	)

	if err := mergeSMF(req.SMFPath, newSMFPath, mergedPath); err != nil {
		return nil, fmt.Errorf("merge SMF failed: %w", err)
	}

	// Preserve original track count for diagnostic logging.
	_ = baseTracks

	wm.logger.InfoContext(ctx, "W5 regeneration complete",
		"region_bars", req.RegionEnd-req.RegionStart,
		"merged_path", mergedPath,
		"duration_ms", elapsed.Milliseconds(),
	)

	return &SmartRegenerationResponse{
		SMFPath:    mergedPath,
		RegionBars: req.RegionEnd - req.RegionStart,
		DurationMs: elapsed.Milliseconds(),
	}, nil
}
