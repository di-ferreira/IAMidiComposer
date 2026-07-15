package server

import (
	"context"
	"fmt"
	"log/slog"
	"math/rand"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"

	aimidi_v1 "aimidi.composer/ace/internal/proto/aimidi/v1"
)

// CompositionServer implements aimidi.v1.CompositionServiceServer.
type CompositionServer struct {
	aimidi_v1.UnimplementedCompositionServiceServer
	logger    *slog.Logger
	mteCliPath string
	tmpDir     string
}

// NewCompositionServer builds a CompositionServer.
func NewCompositionServer(logger *slog.Logger) *CompositionServer {
	mtePath := os.Getenv("MTE_CLI_PATH")
	if mtePath == "" {
		mtePath = "mte_cli"
	}
	tmpDir, err := os.MkdirTemp("", "aimidi-mte-*")
	if err != nil {
		logger.Warn("failed to create temp dir, using /tmp", "error", err)
		tmpDir = "/tmp"
	}
	return &CompositionServer{
		logger:    logger,
		mteCliPath: mtePath,
		tmpDir:     tmpDir,
	}
}

// parsePrompt extracts musical parameters from a natural language prompt.
func parsePrompt(prompt string) (key, scale string, bpm, bars int) {
	key = "C"
	scale = "major"
	bpm = 120
	bars = 4

	lower := strings.ToLower(prompt)

	// Key detection
	keyWords := []string{"c", "d", "e", "f", "g", "a", "b"}
	for _, kw := range keyWords {
		if strings.Contains(lower, " "+kw+" ") || strings.HasPrefix(lower, kw+" ") || strings.Contains(lower, " in "+kw+" ") {
			key = strings.ToUpper(kw)
			break
		}
	}
	// Check for sharps/flats
	if strings.Contains(lower, " sharp") || strings.Contains(lower, "#") {
		key += "#"
	}
	if strings.Contains(lower, " flat") || strings.Contains(lower, "b ") {
		key += "b"
	}

	// Scale detection
	scaleWords := []string{"major", "minor", "dorian", "phrygian", "lydian", "mixolydian", "aeolian", "locrian"}
	for _, sw := range scaleWords {
		if strings.Contains(lower, sw) {
			scale = sw
			break
		}
	}

	// BPM detection
	words := strings.Fields(lower)
	for i, w := range words {
		if w == "bpm" && i > 0 {
			if n, err := strconv.Atoi(words[i-1]); err == nil && n >= 40 && n <= 300 {
				bpm = n
			}
		}
	}

	// Bar detection
	for i, w := range words {
		if (w == "bars" || w == "bar") && i > 0 {
			if n, err := strconv.Atoi(words[i-1]); err == nil && n >= 1 && n <= 64 {
				bars = n
			}
		}
	}

	return
}

// NewComposition implements Workflow 1 (New Composition).
func (s *CompositionServer) NewComposition(ctx context.Context, req *aimidi_v1.NewCompositionRequest) (*aimidi_v1.CompositionResponse, error) {
	seed := req.Seed
	if seed == 0 {
		seed = int64(rand.Uint64())
	}

	key, scale, bpm, bars := parsePrompt(req.Prompt)
	if req.StyleHint != "" {
		parts := strings.Fields(req.StyleHint)
		if len(parts) >= 1 {
			key = parts[0]
		}
		if len(parts) >= 2 {
			scale = parts[1]
		}
	}

	s.logger.InfoContext(ctx, "rpc.NewComposition",
		"seed", seed,
		"prompt", req.Prompt,
		"key", key,
		"scale", scale,
		"bpm", bpm,
		"bars", bars,
	)

	outputFile := filepath.Join(s.tmpDir, fmt.Sprintf("output_%d.mid", seed))
	// Clean up previous output
	os.Remove(outputFile)

	args := []string{
		"--seed", fmt.Sprintf("%d", seed),
		"--key", key,
		"--scale", scale,
		"--bpm", fmt.Sprintf("%d", bpm),
		"--bars", fmt.Sprintf("%d", bars),
		"--output", outputFile,
	}

	cmd := exec.CommandContext(ctx, s.mteCliPath, args...)
	output, err := cmd.CombinedOutput()
	if err != nil {
		s.logger.ErrorContext(ctx, "mte_cli failed",
			"error", err,
			"output", string(output),
		)
		return &aimidi_v1.CompositionResponse{
			Success: false,
			Error:   fmt.Sprintf("mte_cli error: %v - %s", err, string(output)),
		}, nil
	}

	smfData, err := os.ReadFile(outputFile)
	if err != nil {
		s.logger.ErrorContext(ctx, "failed to read SMF output",
			"error", err,
		)
		return &aimidi_v1.CompositionResponse{
			Success: false,
			Error:   fmt.Sprintf("failed to read SMF: %v", err),
		}, nil
	}

	s.logger.InfoContext(ctx, "composition generated",
		"smf_size", len(smfData),
		"output_file", outputFile,
	)

	return &aimidi_v1.CompositionResponse{
		Success:  true,
		SmfMidi:  smfData,
		Error:    "",
	}, nil
}

// ContinueComposition is Workflow 4 (Continue Composition). Stub.
func (s *CompositionServer) ContinueComposition(ctx context.Context, req *aimidi_v1.ContinueCompositionRequest) (*aimidi_v1.CompositionResponse, error) {
	s.logger.InfoContext(ctx, "rpc.ContinueComposition",
		"seed", req.Seed,
		"shared_context_id", req.SharedContextId,
		"from_bar", req.FromBar,
		"bars_to_append", req.BarsToAppend,
	)
	return &aimidi_v1.CompositionResponse{
		Success: false,
		Error:   "not implemented: ContinueComposition (W4 pending; Sprint 8 M2 vertical spike)",
	}, nil
}

// GenerateVariations is Workflow 6 (Generate Variations). Stub.
func (s *CompositionServer) GenerateVariations(ctx context.Context, req *aimidi_v1.GenerateVariationsRequest) (*aimidi_v1.CompositionResponse, error) {
	s.logger.InfoContext(ctx, "rpc.GenerateVariations",
		"base_seed", req.BaseSeed,
		"shared_context_id", req.SharedContextId,
		"variation_count", req.VariationCount,
		"seed_delta", req.SeedSeedDelta,
	)
	return &aimidi_v1.CompositionResponse{
		Success: false,
		Error:   "not implemented: GenerateVariations (W6 pending; Sprint 8 M2 vertical spike)",
	}, nil
}
