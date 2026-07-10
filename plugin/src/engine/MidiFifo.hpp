// MidiFifo — SPSC lock-free ring buffer for MIDI events.
//
// Producer: Message Thread (or a bridge worker) pushes events after the engine
//   returns MIDI. Allowance: allocation is NOT allowed here either (the events
//   come from a pre-decoded arena), but `push` itself never allocates.
// Consumer: Audio Thread drains events inside `processBlock`.
//
// Thread contract:
//   - `push`        -> Message Thread only  (one producer)
//   - `pop`/`size`/`empty`/`clear` -> Audio Thread only (one consumer)
//   - `clear`        may also be called from the Message Thread in
//     `prepareToPlay`/`releaseResources` (which are NOT the audio thread); it
//     only writes `read_pos` which is consumer-owned, so that's safe.
//
// Memory ordering:
//   - `write_pos_` written with `release`, read with `acquire`  (publishes data)
//   - `read_pos_`  written with `release`, read with `acquire`  (publishes drain)
//   - `relaxed` load of the same-side index (no cross-thread dependency on it)
//
// The buffer is power-of-two, so `index & (Capacity - 1)` wraps with no branch.
// Both atomics are cache-line aligned (64 B) to avoid false sharing between the
// producer and consumer CPUs.
//
// Capacity is 4096 for the standard alias (see bottom) — enough for several
// blocks of dense MIDI at typical host buffer sizes (32..1024 samples).
//
// ---
// Self-test: compile as a standalone binary with
//   g++ -std=c++20 -DAIMIDI_MIDI_FIFO_SELF_TEST MidiFifo.hpp -o fifo_test && ./fifo_test
// This satisfies the DoD (unit test + smoke) without pulling GoogleTest into the
// plugin target.
#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <cstdio>

namespace aimidi::plugin {

// A single MIDI event in the FIFO. POD — trivially copyable, no heap.
struct FifoMidiEvent {
    std::uint32_t sample_position; // within the current block (0..samplesPerBlock-1)
    std::uint8_t  channel;         // 0..15
    std::uint8_t  note;            // 0..127
    std::uint8_t  velocity;        // 0..127
    bool          is_note_on;      // true = NoteOn, false = NoteOff
};

// SPSC ring buffer. `Capacity` MUST be a power of two.
template <std::size_t Capacity>
class MidiFifo {
    static_assert((Capacity & (Capacity - 1)) == 0,
                  "Capacity must be power of 2");
    static_assert(Capacity >= 2, "Capacity must be >= 2");

public:
    static constexpr std::size_t capacity = Capacity;

    MidiFifo() = default;

    MidiFifo(const MidiFifo&) = delete;
    MidiFifo& operator=(const MidiFifo&) = delete;
    // Not movable: contains atomics + a fixed array.
    MidiFifo(MidiFifo&&) = delete;
    MidiFifo& operator=(MidiFifo&&) = delete;

    // Producer side (Message Thread). Returns false if the ring is FULL — the
    // caller decides whether to drop or back-pressure. `noexcept` because there
    // is no allocation or locking; a failure on the audio side would be fatal.
    bool push(const FifoMidiEvent& ev) noexcept {
        const auto w = write_pos_.load(std::memory_order_relaxed);
        const auto r = read_pos_.load(std::memory_order_acquire);
        if ((w - r) >= Capacity) {
            return false; // full — drop
        }
        buffer_[w & (Capacity - 1)] = ev;
        write_pos_.store(w + 1, std::memory_order_release);
        return true;
    }

    // Consumer side (Audio Thread). Returns false if empty.
    bool pop(FifoMidiEvent& out) noexcept {
        const auto r = read_pos_.load(std::memory_order_relaxed);
        const auto w = write_pos_.load(std::memory_order_acquire);
        if (r == w) {
            return false; // empty
        }
        out = buffer_[r & (Capacity - 1)];
        read_pos_.store(r + 1, std::memory_order_release);
        return true;
    }

    // Consumer-visible count. Safe to call from either thread (acquire on both
    // indices), though the audio-thread value is the one that matters.
    [[nodiscard]] std::size_t size() const noexcept {
        const auto w = write_pos_.load(std::memory_order_acquire);
        const auto r = read_pos_.load(std::memory_order_acquire);
        return w - r;
    }

    [[nodiscard]] bool empty() const noexcept { return size() == 0; }

    // Resets the consumer view to empty. Only writes `read_pos_`; the producer's
    // `write_pos_` is left untouched, which is fine because once read==write the
    // ring looks empty and subsequent pushes will start from a fresh slot.
    // Safe to call from prepareToPlay / releaseResources (NOT the audio thread,
    // though audio-thread callers would also be correct in a single-consumer
    // scenario — we forbid it simply to keep the contract clean).
    void clear() noexcept {
        read_pos_.store(write_pos_.load(std::memory_order_acquire),
                        std::memory_order_release);
    }

private:
    std::array<FifoMidiEvent, Capacity> buffer_{};

    alignas(64) std::atomic<std::size_t> write_pos_{0}; // producer-owned
    alignas(64) std::atomic<std::size_t> read_pos_{0};  // consumer-owned
};

// Default capacity — a few blocks of dense MIDI at typical host buffer sizes.
using StandardMidiFifo = MidiFifo<4096>;

} // namespace aimidi::plugin

// ---- Self-test (opt-in, no GoogleTest) ------------------------------------
// Compile with: g++ -std=c++20 -DAIMIDI_MIDI_FIFO_SELF_TEST MidiFifo.hpp -o fifo_test
#ifdef AIMIDI_MIDI_FIFO_SELF_TEST
#include <cassert>

int main() {
    using namespace aimidi::plugin;

    MidiFifo<4> fifo; // smallest power-of-two that exercises wrap
    static_assert(MidiFifo<4>::capacity == 4);

    FifoMidiEvent ev{};

    // 1) pop on empty returns false
    assert(!fifo.empty() == false);
    assert(fifo.empty());
    assert(!fifo.pop(ev));

    // 2) push then pop returns the same event
    ev.sample_position = 7;
    ev.channel = 0;
    ev.note = 60;
    ev.velocity = 100;
    ev.is_note_on = true;
    assert(fifo.push(ev));
    assert(!fifo.empty());
    FifoMidiEvent out{};
    assert(fifo.pop(out));
    assert(out.sample_position == 7);
    assert(out.channel == 0);
    assert(out.note == 60);
    assert(out.velocity == 100);
    assert(out.is_note_on);
    assert(fifo.empty());
    assert(!fifo.pop(out));

    // 3) fill up to Capacity (4 slots), the next push fails (FULL)
    for (std::size_t i = 0; i < MidiFifo<4>::capacity; ++i) {
        ev.sample_position = static_cast<std::uint32_t>(i);
        assert(fifo.push(ev));
    }
    // ring reports full
    assert(fifo.size() == MidiFifo<4>::capacity);
    assert(!fifo.push(ev)); // overflow — drop

    // 4) drain all Capacity entries again, in order
    for (std::size_t i = 0; i < MidiFifo<4>::capacity; ++i) {
        assert(fifo.pop(out));
        assert(out.sample_position == static_cast<std::uint32_t>(i));
    }
    assert(fifo.empty());

    // 5) wrap-around: push/pop more than Capacity in interleaved fashion
    for (std::size_t i = 0; i < 16; ++i) {
        ev.sample_position = static_cast<std::uint32_t>(i);
        assert(fifo.push(ev));
        assert(fifo.pop(out));
        assert(out.sample_position == static_cast<std::uint32_t>(i));
    }
    assert(fifo.empty());

    // 6) fill, clear, then assert empty
    for (std::size_t i = 0; i < MidiFifo<4>::capacity; ++i) {
        assert(fifo.push(ev));
    }
    assert(!fifo.empty());
    fifo.clear();
    assert(fifo.empty());
    assert(!fifo.pop(out));

    // 7) StandardMidiFifo smoke (just a couple of ops on the production alias)
    StandardMidiFifo big;
    ev.sample_position = 12345;
    assert(big.push(ev));
    assert(big.pop(out));
    assert(out.sample_position == 12345);
    assert(big.empty());

    std::printf("OK\n");
    return 0;
}
#endif // AIMIDI_MIDI_FIFO_SELF_TEST
