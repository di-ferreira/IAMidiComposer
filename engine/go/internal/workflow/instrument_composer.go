package workflow

import (
	"context"
	"fmt"
	"os"
	"time"
)

// InstrumentComposerRequest represents W2 input.
type InstrumentComposerRequest struct {
	BlueprintJSON string
	Seed          int64
	Instrument    string // e.g. "piano", "bass", "drums"
	OutputPath    string // optional; empty = temp file
}

// InstrumentComposerResponse represents W2 output.
type InstrumentComposerResponse struct {
	SMFPath    string
	EventCount int
	DurationMs int64
}

// InstrumentComposer implements Workflow 2 (Instrument Composer).
//
// It generates MIDI for a single instrument from the blueprint by passing
// --instrument to mte_cli.
func (wm *WorkflowManager) InstrumentComposer(ctx context.Context, req *InstrumentComposerRequest) (*InstrumentComposerResponse, error) {
	if req.Instrument == "" {
		return nil, fmt.Errorf("instrument must not be empty")
	}

	wm.logger.InfoContext(ctx, "W2.InstrumentComposer",
		"seed", req.Seed,
		"instrument", req.Instrument,
	)

	args := []string{
		"--blueprint-json", req.BlueprintJSON,
		"--instrument", req.Instrument,
	}

	start := time.Now()
	smfPath, err := wm.runMTE(ctx, args, req.Seed)
	elapsed := time.Since(start)
	if err != nil {
		return nil, fmt.Errorf("instrument composer failed: %w", err)
	}

	if req.OutputPath != "" {
		data, err := os.ReadFile(smfPath)
		if err != nil {
			return nil, fmt.Errorf("read generated SMF: %w", err)
		}
		if err := os.WriteFile(req.OutputPath, data, 0644); err != nil {
			return nil, fmt.Errorf("copy SMF to output path: %w", err)
		}
		_ = os.Remove(smfPath)
		smfPath = req.OutputPath
	}

	// Estimate event count from file size (rough: ~4 bytes per event).
	info, err := os.Stat(smfPath)
	eventCount := 0
	if err == nil {
		eventCount = int(info.Size() / 4)
	}

	wm.logger.InfoContext(ctx, "W2 instrument generated",
		"path", smfPath,
		"events_est", eventCount,
		"duration_ms", elapsed.Milliseconds(),
	)

	return &InstrumentComposerResponse{
		SMFPath:    smfPath,
		EventCount: eventCount,
		DurationMs: elapsed.Milliseconds(),
	}, nil
}
