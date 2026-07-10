// Package main is the AI Composition Engine (ACE) entrypoint.
//
// The ACE is a gRPC server listening on a local socket. Plugin VST3
// connects to it (see ADR-0001) and the Music Theory Engine (C++) will
// also be a downstream gRPC caller once ADR-0004 lands. Workflow handlers
// are stubs in Sprint 2 — real MTE wiring starts in Sprint 4 (Fase 2
// of ROADMAP.md).
package main

import (
	"context"
	"log/slog"
	"os"
	"os/signal"
	"runtime"
	"syscall"

	"aimidi.composer/ace/internal/server"
)

// defaultListenerAddr is unix socket on POSIX and tcp on Windows
// (env override via AIMIDI_LISTEN_ADDR).
func defaultListenerAddr() string {
	if runtime.GOOS == "windows" {
		return "127.0.0.1:50051"
	}
	return "/tmp/aimidi-mte.sock"
}

func listenerAddr() string {
	if v := os.Getenv("AIMIDI_LISTEN_ADDR"); v != "" {
		return v
	}
	return defaultListenerAddr()
}

func main() {
	logger := slog.New(slog.NewTextHandler(os.Stderr, &slog.HandlerOptions{Level: slog.LevelInfo}))
	logger.Info("AI Composition Engine (ACE) booting",
		"version", "0.1.0",
		"build", "sprint2",
		"listen", listenerAddr(),
	)
	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()
	srv := server.New(listenerAddr(), logger)
	if err := srv.Serve(ctx); err != nil {
		logger.Error("ACE server terminated with error", "err", err)
		os.Exit(1)
	}
	logger.Info("ACE shutdown complete")
}
