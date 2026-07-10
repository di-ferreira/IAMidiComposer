package server

import (
	"context"
	"errors"
	"fmt"
	"log/slog"
	"net"
	"os"
	"strings"

	"golang.org/x/sync/errgroup"
	"google.golang.org/grpc"

	aimidi_v1 "aimidi.composer/ace/internal/proto/aimidi/v1"
)

// GRPCServer wraps a *grpc.Server, registering CompositionService and
// owning its listener address. It is constructed via New and started
// via Serve; graceful shutdown is driven by cancelling the context
// passed to Serve (signal.NotifyContext in main).
type GRPCServer struct {
	srv    *grpc.Server
	addr   string
	logger *slog.Logger
}

// New creates a GRPCServer registered with CompositionService. The
// addr is resolved by Serve: a path starting with "/" or "unix:" uses
// unix domain socket (POSIX); everything else uses TCP — Windows
// callers should pass a "host:port" address.
func New(addr string, logger *slog.Logger) *GRPCServer {
	s := grpc.NewServer()
	aimidi_v1.RegisterCompositionServiceServer(s, NewCompositionServer(logger))
	return &GRPCServer{srv: s, addr: addr, logger: logger}
}

// Serve binds the listener and serves gRPC requests until ctx is
// cancelled (SIGINT/SIGTERM) or the server stops. On context
// cancellation it triggers GracefulStop and returns nil on a clean
// shutdown, or the serve error otherwise.
func (g *GRPCServer) Serve(ctx context.Context) error {
	network, addr := resolveListener(g.addr)

	if network == "unix" {
		if err := os.Remove(addr); err != nil && !errors.Is(err, os.ErrNotExist) {
			return fmt.Errorf("removing stale unix socket %q: %w", addr, err)
		}
	}

	lis, err := net.Listen(network, addr)
	if err != nil {
		return fmt.Errorf("listen %s://%s: %w", network, addr, err)
	}
	g.logger.Info("listening on " + network + "://" + addr)

	eg, egctx := errgroup.WithContext(ctx)
	eg.Go(func() error {
		return g.srv.Serve(lis)
	})
	eg.Go(func() error {
		<-egctx.Done()
		g.logger.Info("graceful shutdown triggered")
		g.srv.GracefulStop()
		return nil
	})

	serveErr := eg.Wait()
	if serveErr != nil && !errors.Is(serveErr, grpc.ErrServerStopped) {
		g.logger.Error("gRPC server stopped with error", "err", serveErr)
		return serveErr
	}
	g.logger.Info("graceful shutdown complete")
	return nil
}

// resolveListener maps the logical address to a (network, address)
// pair understood by net.Listen. "unix:" prefixed addresses and bare
// filesystem paths (starting with "/") are treated as unix domain
// sockets; everything else is TCP.
func resolveListener(addr string) (network, resolved string) {
	switch {
	case strings.HasPrefix(addr, "unix:"):
		return "unix", strings.TrimPrefix(addr, "unix:")
	case strings.HasPrefix(addr, "/"):
		return "unix", addr
	default:
		return "tcp", addr
	}
}
