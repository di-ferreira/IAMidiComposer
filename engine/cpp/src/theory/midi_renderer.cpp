// Implementation: MidiRenderer — SMF Type 1 output.
//
// Converts tick-based MidiEvents into a complete .mid byte stream.
// Big-endian, VLQ-encoded delta times, tempo track first.
#include <aimidi/theory/IMidiRenderer.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace aimidi::theory {

namespace {

// ------------------------------------------------------------------
// Big-endian helpers
// ------------------------------------------------------------------
void write_be16(std::vector<std::uint8_t>& out, std::uint16_t v) {
    out.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<std::uint8_t>(v & 0xFF));
}

void write_be32(std::vector<std::uint8_t>& out, std::uint32_t v) {
    out.push_back(static_cast<std::uint8_t>((v >> 24) & 0xFF));
    out.push_back(static_cast<std::uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<std::uint8_t>(v & 0xFF));
}

// ------------------------------------------------------------------
// Variable Length Quantity
// ------------------------------------------------------------------
std::vector<std::uint8_t> encode_vlq(std::uint32_t value) {
    std::vector<std::uint8_t> bytes;
    bytes.push_back(static_cast<std::uint8_t>(value & 0x7F));
    value >>= 7;
    while (value > 0) {
        bytes.push_back(static_cast<std::uint8_t>((value & 0x7F) | 0x80));
        value >>= 7;
    }
    std::reverse(bytes.begin(), bytes.end());
    return bytes;
}

// ------------------------------------------------------------------
// Key signature: map key name string -> sharps/flats count
// ------------------------------------------------------------------
int key_sharps(const std::string& key) {
    static const std::map<std::string, int, std::less<>> kKeyMap = {
        {"Cb", -7}, {"Gb", -6}, {"Db", -5}, {"Ab", -4}, {"Eb", -3},
        {"Bb", -2}, {"F",  -1}, {"C",   0}, {"G",   1}, {"D",   2},
        {"A",   3}, {"E",   4}, {"B",   5}, {"F#",  6}, {"C#",  7},
    };
    auto it = kKeyMap.find(key);
    return (it != kKeyMap.end()) ? it->second : 0;
}

// ------------------------------------------------------------------
// Internal wire-format event for sorting
// ------------------------------------------------------------------
struct WireEvent {
    std::uint32_t tick;
    bool          is_note_off; // true = note off (sent before note on at same tick)
    std::uint8_t  status;
    std::uint8_t  data1;
    std::uint8_t  data2;
};

} // namespace

class MidiRenderer final : public IMidiRenderer {
public:
    std::vector<std::uint8_t> render(const SmfComposition& comp) const override {
        std::vector<std::uint8_t> out;
        out.reserve(4096);

        const int total_tracks = static_cast<int>(comp.tracks.size()) + 1; // +1 tempo

        // ---- MThd chunk ----
        out.insert(out.end(), {'M', 'T', 'h', 'd'});
        write_be32(out, 6u);                        // chunk length
        write_be16(out, 1);                         // format 1
        write_be16(out, static_cast<std::uint16_t>(total_tracks));
        write_be16(out, static_cast<std::uint16_t>(comp.ppq));

        // ---- Tempo track (first) ----
        build_tempo_track(out, comp);

        // ---- Instrument tracks ----
        for (const auto& track : comp.tracks) {
            build_instrument_track(out, track, comp);
        }

        return out;
    }

private:
    // ------------------------------------------------------------------
    // Tempo track
    // ------------------------------------------------------------------
    void build_tempo_track(std::vector<std::uint8_t>& out,
                           const SmfComposition& comp) const {
        std::vector<std::uint8_t> events;
        events.reserve(64);

        // Tempo meta event: FF 51 03 tt tt tt
        events.push_back(0x00); // delta = 0
        events.push_back(0xFF); events.push_back(0x51);
        events.push_back(0x03);
        const std::uint32_t tempo_us = 60'000'000u / static_cast<std::uint32_t>(comp.bpm);
        events.push_back(static_cast<std::uint8_t>((tempo_us >> 16) & 0xFF));
        events.push_back(static_cast<std::uint8_t>((tempo_us >> 8) & 0xFF));
        events.push_back(static_cast<std::uint8_t>(tempo_us & 0xFF));

        // Time signature meta event: FF 58 04 nn dd cc bb
        events.push_back(0x00); // delta = 0
        events.push_back(0xFF); events.push_back(0x58);
        events.push_back(0x04);
        events.push_back(static_cast<std::uint8_t>(comp.time_sig_num));
        // denominator as power of 2: 4 -> 2, 8 -> 3, etc.
        {
            int dd = 0;
            int den = comp.time_sig_den;
            while (den > 1) { den >>= 1; ++dd; }
            events.push_back(static_cast<std::uint8_t>(dd));
        }
        events.push_back(24); // clocks per tick
        events.push_back(8);  // 32nd notes per quarter

        // Key signature meta event: FF 59 02 sf mi
        events.push_back(0x00); // delta = 0
        events.push_back(0xFF); events.push_back(0x59);
        events.push_back(0x02);
        {
            const int sf = key_sharps(comp.key);
            events.push_back(static_cast<std::uint8_t>(static_cast<int8_t>(sf)));
        }
        events.push_back((comp.scale == "minor" || comp.scale == "min") ? 1u : 0u);

        // End of track: FF 2F 00
        events.push_back(0x00); // delta = 0
        events.push_back(0xFF); events.push_back(0x2F);
        events.push_back(0x00);

        // Write MTrk chunk
        out.insert(out.end(), {'M', 'T', 'r', 'k'});
        write_be32(out, static_cast<std::uint32_t>(events.size()));
        out.insert(out.end(), events.begin(), events.end());
    }

    // ------------------------------------------------------------------
    // Instrument track
    // ------------------------------------------------------------------
    void build_instrument_track(std::vector<std::uint8_t>& out,
                                const MidiTrack& track,
                                const SmfComposition& /*comp*/) const {
        // Build wire events
        std::vector<WireEvent> wire;
        wire.reserve(track.events.size() * 2 + 1);

        // Track name meta event (delta=0) — written directly before wire events
        std::vector<std::uint8_t> name_bytes;
        {
            auto vlq = encode_vlq(0);
            name_bytes.insert(name_bytes.end(), vlq.begin(), vlq.end());
            name_bytes.push_back(0xFF); name_bytes.push_back(0x03);
            name_bytes.push_back(static_cast<std::uint8_t>(track.name.size()));
            name_bytes.insert(name_bytes.end(), track.name.begin(), track.name.end());
        }

        // Convert MidiEvents to note-on/note-off pairs
        for (const auto& ev : track.events) {
            if (ev.note > 127) continue;

            // Note on at tick_on
            WireEvent on{};
            on.tick = ev.tick_on;
            on.is_note_off = false;
            on.status = static_cast<std::uint8_t>(0x90 | (ev.channel & 0x0F));
            on.data1 = ev.note;
            on.data2 = ev.velocity;
            wire.push_back(on);

            // Note off at tick_off
            WireEvent off{};
            off.tick = ev.tick_off;
            off.is_note_off = true;
            off.status = static_cast<std::uint8_t>(0x80 | (ev.channel & 0x0F));
            off.data1 = ev.note;
            off.data2 = ev.velocity;
            wire.push_back(off);
        }

        // Sort wire events by tick, note_off before note_on at same tick
        std::sort(wire.begin(), wire.end(),
                  [](const WireEvent& a, const WireEvent& b) {
                      if (a.tick != b.tick) return a.tick < b.tick;
                      // note_off (true) before note_on (false)
                      return a.is_note_off && !b.is_note_off;
                  });

        // Build event bytes
        std::vector<std::uint8_t> track_events;
        track_events.reserve(name_bytes.size() + wire.size() * 4 + 4);

        // Write track name first
        track_events.insert(track_events.end(), name_bytes.begin(), name_bytes.end());

        // Write wire events with delta times
        std::uint32_t last_tick = 0;

        // Track running status for compression (optional, but good practice)
        std::uint8_t last_status = 0;

        for (const auto& we : wire) {
            const std::uint32_t delta = we.tick - last_tick;
            last_tick = we.tick;

            auto vlq = encode_vlq(delta);
            track_events.insert(track_events.end(), vlq.begin(), vlq.end());

            // Running status: if same status byte, omit it
            if (we.status != last_status || last_status == 0) {
                track_events.push_back(we.status);
                last_status = we.status;
            }
            track_events.push_back(we.data1);
            track_events.push_back(we.data2);
        }

        // End of track
        {
            auto vlq = encode_vlq(0);
            track_events.insert(track_events.end(), vlq.begin(), vlq.end());
            track_events.push_back(0xFF); track_events.push_back(0x2F);
            track_events.push_back(0x00);
        }

        // Write MTrk chunk
        out.insert(out.end(), {'M', 'T', 'r', 'k'});
        write_be32(out, static_cast<std::uint32_t>(track_events.size()));
        out.insert(out.end(), track_events.begin(), track_events.end());
    }
};

std::unique_ptr<IMidiRenderer> make_midi_renderer() {
    return std::make_unique<MidiRenderer>();
}

} // namespace aimidi::theory
