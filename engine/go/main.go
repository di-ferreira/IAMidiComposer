// Package main is the AI Composition Engine entrypoint.
//
// Skeleton: the actual gRPC server wiring will be added once the proto codegen
// (buf/protoc) is configured and the C++ Music Theory Engine binding (cgo or
// internal gRPC) is decided in ADR-0001.
package main

import (
	"log/slog"
	"os"
)

func main() {
	logger := slog.New(slog.NewTextHandler(os.Stderr, &slog.HandlerOptions{
		Level: slog.LevelInfo,
	}))
	logger.Info("AI Composition Engine (ACE) booting",
		"version", "0.1.0",
		"build", "skeleton",
	)
	// TODO: grpc server listening on unix socket or localhost:50051
	// TODO: wire CompositionService handlers via DI
	// TODO:start MI/MTE/Audio/Instrument subagents via internal gRPC or cgo
	select {} // block
}
