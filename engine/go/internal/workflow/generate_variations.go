package workflow

import (
	"context"
	"fmt"
	"os"
	"time"
)

// GenerateVariationsRequest represents W6 input.
type GenerateVariationsRequest struct {
	BlueprintJSON     string
	BaseSeed          int64
	VariationCount    int
	DeltaPerVariation int64
}

// GenerateVariationsResponse represents W6 output.
type GenerateVariationsResponse struct {
	Variations []VariationResult
}

// VariationResult holds the result of a single variation.
type VariationResult struct {
	Index      int
	Seed       int64
	SMFPath    string
	DurationMs int64
}

// GenerateVariations implements Workflow 6 (Generate Variations).
//
// It calls mte_cli for each variation, varying only the seed while keeping
// the same blueprint. The blueprint is passed as --blueprint-json.
func (wm *WorkflowManager) GenerateVariations(ctx context.Context, req *GenerateVariationsRequest) (*GenerateVariationsResponse, error) {
	if req.VariationCount <= 0 {
		return nil, fmt.Errorf("variation_count must be > 0, got %d", req.VariationCount)
	}
	if req.DeltaPerVariation == 0 {
		req.DeltaPerVariation = 1
	}

	wm.logger.InfoContext(ctx, "W6.GenerateVariations",
		"base_seed", req.BaseSeed,
		"count", req.VariationCount,
		"delta", req.DeltaPerVariation,
	)

	resp := &GenerateVariationsResponse{
		Variations: make([]VariationResult, 0, req.VariationCount),
	}

	for i := range req.VariationCount {
		seed := req.BaseSeed + (int64(i) * req.DeltaPerVariation)

		args := []string{
			"--blueprint-json", req.BlueprintJSON,
		}

		start := time.Now()
		smfPath, err := wm.runMTE(ctx, args, seed)
		elapsed := time.Since(start)

		if err != nil {
			wm.logger.ErrorContext(ctx, "W6 variation failed",
				"index", i,
				"seed", seed,
				"error", err,
			)
			// Keep going — partial results are still useful.
			resp.Variations = append(resp.Variations, VariationResult{
				Index: i,
				Seed:  seed,
			})
			continue
		}

		// Defer cleanup of the individual variation file; the caller
		// should copy the file before the response goes out of scope.
		defer func(path string) {
			_ = os.Remove(path)
		}(smfPath)

		wm.logger.InfoContext(ctx, "W6 variation generated",
			"index", i,
			"seed", seed,
			"path", smfPath,
			"duration_ms", elapsed.Milliseconds(),
		)

		resp.Variations = append(resp.Variations, VariationResult{
			Index:      i,
			Seed:       seed,
			SMFPath:    smfPath,
			DurationMs: elapsed.Milliseconds(),
		})
	}

	return resp, nil
}
