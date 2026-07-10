// Stub Engine Bridge — returns failure for every call (Sprint 3 placeholder).
//
// The real `GrpcEngineBridge` implementation will be added once ADR-0004 is
// decided (lib estatica vs processo separado). Until then this stub keeps the
// plugin building without any gRPC/protobuf dependency.
//
// Thread contract: all methods run on the **Message Thread** (or a bridge worker
// thread). The Audio Thread NEVER touches this object.
#include "EngineBridge.hpp"

#include <juce_core/juce_core.h>

namespace aimidi::plugin {

namespace {

class StubEngineBridge final : public IEngineBridge {
public:
    CompositionResult newComposition(const CompositionRequest& req) override {
        juce::ignoreUnused(req);
        CompositionResult r;
        r.success = false;
        r.error = "stub: bridge not wired (Sprint 3; gRPC client comes after ADR-0004)";
        return r;
    }

    CompositionResult continueComposition(const CompositionRequest& req) override {
        juce::ignoreUnused(req);
        CompositionResult r;
        r.success = false;
        r.error = "stub: continueComposition not wired (Sprint 3; gRPC client comes after ADR-0004)";
        return r;
    }

    CompositionResult generateVariations(const CompositionRequest& req) override {
        juce::ignoreUnused(req);
        CompositionResult r;
        r.success = false;
        r.error = "stub: generateVariations not wired (Sprint 3; gRPC client comes after ADR-0004)";
        return r;
    }

    [[nodiscard]] bool isConnected() const noexcept override { return false; }

    void connect(std::string_view /*address*/) override {
        // no-op — stub never connects
    }

    void disconnect() noexcept override {
        // no-op
    }
};

} // namespace

std::unique_ptr<IEngineBridge> make_stub_engine_bridge() {
    return std::make_unique<StubEngineBridge>();
}

} // namespace aimidi::plugin
