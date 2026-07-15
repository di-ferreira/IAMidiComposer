package workflow

import (
	"context"
	"fmt"
	"time"
)

// ArrangeRequest represents W10 input.
//
// Arrangement reinterprets a chord progression from the blueprint as a
// complete rhythm-section + pad arrangement suitable for the given style.
type ArrangeRequest struct {
	BlueprintJSON string
	Seed          int64
	Style         string // "pop", "rock", "jazz", "electronic"
	OutputPath    string // optional; empty = temp file
}

// ArrangeResponse represents W10 output.
type ArrangeResponse struct {
	SMFPath    string
	TrackCount int
	DurationMs int64
}

// Arrange implements Workflow 10 (Arrange).
//
// It parses the blueprint for the chord progression, calls mte_cli with
// --arrange and the target style, and returns an SMF file containing a
// full rhythm-section arrangement (keys, guitar, bass, drums) plus pad
// layers (strings, pads).
func (wm *WorkflowManager) Arrange(ctx context.Context, req *ArrangeRequest) (*ArrangeResponse, error) {
	if req.BlueprintJSON == "" {
		return nil, fmt.Errorf("blueprint_json must not be empty")
	}
	if req.Style == "" {
		req.Style = "pop"
	}

	wm.logger.InfoContext(ctx, "W10.Arrange",
		"seed", req.Seed,
		"style", req.Style,
	)

	args := []string{
		"--blueprint-json", req.BlueprintJSON,
		"--arrange", req.Style,
	}

	start := time.Now()
	smfPath, err := wm.runMTE(ctx, args, req.Seed)
	elapsed := time.Since(start)
	if err != nil {
		return nil, fmt.Errorf("arrange failed: %w", err)
	}

	if req.OutputPath != "" {
		data, err := os.ReadFile(smfPath)
		if err != nil {
			return nil, fmt.Errorf("read generated SMF: %w", err)
		}
		if err := os.WriteFile(req.OutputPath, data, 0644); err != nil {
			return nil, fmt.Errorf("copy SMF to output path: %w", err)
		}
		os.Remove(smfPath)
		smfPath = req.OutputPath
	}

	// Read back the generated file to count tracks.
	header, tracks, err := readSMF(smfPath)
	trackCount := 0
	if err == nil {
		trackCount = int(header.NumTracks)
	}
	_ = tracks

	wm.logger.InfoContext(ctx, "W10 arrange complete",
		"path", smfPath,
		"tracks", trackCount,
		"duration_ms", elapsed.Milliseconds(),
	)

	return &ArrangeResponse{
		SMFPath:    smfPath,
		TrackCount: trackCount,
		DurationMs: elapsed.Milliseconds(),
	}, nil
}
