// Engine Bridge interface — Plugin <-> ACE boundary (Sprint 3, Phase 1.4).
//
// The plugin depends ONLY on this pure C++ interface (DI / Clean Arch). The real
// gRPC implementation (`GrpcEngineBridge`) will arrive after ADR-0004 decides
// between static lib vs separate process. Until then `StubEngineBridge` keeps the
// plugin buildable with zero extra dependencies.
//
// IMPORTANT: ALL methods here are called on the **Message Thread** (or a worker
// spawned by the bridge). NEVER call these from the Audio Thread — see
// standards/realtime_audio.md.
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace aimidi::plugin {

// Request to start/continue a composition. Allocation is permitted: this struct
// lives on the Message Thread only.
struct CompositionRequest {
    std::uint64_t seed = 0;
    std::string prompt;
    std::string style_hint;
    std::int32_t target_seconds = 0;
};

// Result of a composition call. `smf_midi` holds raw Standard MIDI File bytes
// (alloc OK — Message Thread). The audio thread never touches this struct.
struct CompositionResult {
    bool success = false;
    std::string error;
    std::vector<std::uint8_t> smf_midi;
    std::string shared_context_id;
};

// Pure interface — the plugin owns a unique_ptr<IEngineBridge> and talks to the
// ACE exclusively through it. No gRPC headers leak into the plugin.
class IEngineBridge {
public:
    virtual ~IEngineBridge() = default;

    virtual CompositionResult newComposition(const CompositionRequest& req) = 0;
    virtual CompositionResult continueComposition(const CompositionRequest& req) = 0;
    virtual CompositionResult generateVariations(const CompositionRequest& req) = 0;

    [[nodiscard]] virtual bool isConnected() const noexcept = 0;
    virtual void connect(std::string_view address) = 0;
    virtual void disconnect() noexcept = 0;
};

// Factory declared here, defined in StubEngineBridge.cpp.
std::unique_ptr<IEngineBridge> make_stub_engine_bridge();

} // namespace aimidi::plugin
