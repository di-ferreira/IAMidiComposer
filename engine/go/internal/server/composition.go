package server

import (
	"context"
	"log/slog"

	aimidi_v1 "aimidi.composer/ace/internal/proto/aimidi/v1"
)

// CompositionServer implements aimidi.v1.CompositionServiceServer with
// Sprint 2 stubs: every RPC logs structured fields and returns a
// CompositionResponse{Success:false, Error:"not implemented: ..."}
// so the plugin VST3 can connect and exercise the contract while the
// real Music Theory Engine wiring lands in Sprint 4.
type CompositionServer struct {
	aimidi_v1.UnimplementedCompositionServiceServer
	logger *slog.Logger
}

// NewCompositionServer builds a CompositionServer injecting the logger
// (DI: no package-level state, no globals).
func NewCompositionServer(logger *slog.Logger) *CompositionServer {
	return &CompositionServer{logger: logger}
}

// NewComposition is Workflow 1 (New Composition). Sprint 2 stub.
func (s *CompositionServer) NewComposition(ctx context.Context, req *aimidi_v1.NewCompositionRequest) (*aimidi_v1.CompositionResponse, error) {
	s.logger.InfoContext(ctx, "rpc.NewComposition",
		"seed", req.Seed,
		"prompt_len", len(req.Prompt),
		"style_hint", req.StyleHint,
		"target_seconds", req.TargetSeconds,
	)
	return &aimidi_v1.CompositionResponse{
		Success: false,
		Error:   "not implemented: NewComposition (Sprint 2 stub; MTE wiring in Sprint 4)",
	}, nil
}

// ContinueComposition is Workflow 4 (Continue Composition). Sprint 2 stub.
func (s *CompositionServer) ContinueComposition(ctx context.Context, req *aimidi_v1.ContinueCompositionRequest) (*aimidi_v1.CompositionResponse, error) {
	s.logger.InfoContext(ctx, "rpc.ContinueComposition",
		"seed", req.Seed,
		"shared_context_id", req.SharedContextId,
		"from_bar", req.FromBar,
		"bars_to_append", req.BarsToAppend,
	)
	return &aimidi_v1.CompositionResponse{
		Success: false,
		Error:   "not implemented: ContinueComposition (Sprint 2 stub; MTE wiring in Sprint 4)",
	}, nil
}

// GenerateVariations is Workflow 6 (Generate Variations). Sprint 2 stub.
func (s *CompositionServer) GenerateVariations(ctx context.Context, req *aimidi_v1.GenerateVariationsRequest) (*aimidi_v1.CompositionResponse, error) {
	s.logger.InfoContext(ctx, "rpc.GenerateVariations",
		"base_seed", req.BaseSeed,
		"shared_context_id", req.SharedContextId,
		"variation_count", req.VariationCount,
		"seed_delta", req.SeedSeedDelta,
	)
	return &aimidi_v1.CompositionResponse{
		Success: false,
		Error:   "not implemented: GenerateVariations (Sprint 2 stub; MTE wiring in Sprint 4)",
	}, nil
}
