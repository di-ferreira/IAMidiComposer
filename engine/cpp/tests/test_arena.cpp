// Test: core/MidiEventArena — arena allocator for MidiEvent vectors.
//
// Exercises: default capacity, allocate, allocate_n, reset reuse,
// ArenaEventVector push_back/iteration, STL allocator adapter.
#include <aimidi/core/MidiEventArena.hpp>
#include <aimidi/theory/Types.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

namespace aimidi::core {
namespace {

using aimidi::theory::MidiEvent;

// ─── MidiEventArena ──────────────────────────────────────────────────────

TEST(MidiEventArena_DefaultCapacity, Is4096) {
    MidiEventArena arena;
    EXPECT_EQ(arena.capacity(), 4096u);
    EXPECT_EQ(arena.used(), 0u);
    EXPECT_EQ(arena.remaining(), 4096u);
}

TEST(MidiEventArena_AllocateReturnsNonNull, NonNull) {
    MidiEventArena arena(64);
    auto* ev = arena.allocate();
    ASSERT_NE(ev, nullptr);
}

TEST(MidiEventArena_AllocateReturnsDistinctAddresses, Distinct) {
    MidiEventArena arena(64);
    auto* a = arena.allocate();
    auto* b = arena.allocate();
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    EXPECT_NE(a, b);
}

TEST(MidiEventArena_ResetReclaimsMemory, Reuses) {
    MidiEventArena arena(4);
    auto* a = arena.allocate();
    ASSERT_NE(a, nullptr);
    a->tick_on = 100;
    a->note = 60;
    EXPECT_EQ(arena.used(), 1u);

    arena.reset();
    EXPECT_EQ(arena.used(), 0u);

    auto* b = arena.allocate();
    ASSERT_NE(b, nullptr);
    // After reset the arena reuses the same block, so b may equal a.
    EXPECT_EQ(b, a);
}

TEST(MidiEventArena_AllocateReturnsNullWhenFull, NullOnExhaustion) {
    MidiEventArena arena(2);
    ASSERT_NE(arena.allocate(), nullptr);
    ASSERT_NE(arena.allocate(), nullptr);
    EXPECT_EQ(arena.allocate(), nullptr);
    EXPECT_EQ(arena.remaining(), 0u);
}

TEST(MidiEventArena_AllocateN, ContiguousBlock) {
    MidiEventArena arena(16);
    auto* block = arena.allocate_n(5);
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(arena.used(), 5u);
    EXPECT_EQ(arena.remaining(), 11u);

    block[0].tick_on = 1;
    block[1].tick_on = 2;
    block[2].tick_on = 3;
    EXPECT_EQ(block[0].tick_on, 1u);
    EXPECT_EQ(block[1].tick_on, 2u);
    EXPECT_EQ(block[2].tick_on, 3u);
}

TEST(MidiEventArena_AllocateN, ReturnsNullWhenExhausted) {
    MidiEventArena arena(3);
    EXPECT_EQ(arena.allocate_n(4), nullptr);
    EXPECT_EQ(arena.used(), 0u);
}

// ─── ArenaEventVector ────────────────────────────────────────────────────

TEST(ArenaEventVector_PushBack, StoresCorrectValues) {
    MidiEventArena arena(16);
    ArenaEventVector vec(arena);

    const MidiEvent ev1{ .tick_on = 0, .tick_off = 120, .channel = 0,
                         .note = 60, .velocity = 100, .articulation = 0 };
    const MidiEvent ev2{ .tick_on = 120, .tick_off = 240, .channel = 0,
                         .note = 64, .velocity = 90, .articulation = 1 };

    EXPECT_TRUE(vec.push_back(ev1));
    EXPECT_TRUE(vec.push_back(ev2));
    ASSERT_EQ(vec.size(), 2u);

    EXPECT_EQ(vec[0], ev1);
    EXPECT_EQ(vec[1], ev2);
}

TEST(ArenaEventVector_PushBack, ReturnsFalseWhenFull) {
    MidiEventArena arena(1);
    ArenaEventVector vec(arena);
    EXPECT_TRUE(vec.push_back({}));
    EXPECT_FALSE(vec.push_back({}));
    EXPECT_EQ(vec.size(), 1u);
}

TEST(ArenaEventVector_Iteration, RangeForWorks) {
    MidiEventArena arena(16);
    ArenaEventVector vec(arena);

    vec.push_back({ .tick_on = 0, .tick_off = 0, .channel = 0,
                    .note = 60, .velocity = 100, .articulation = 0 });
    vec.push_back({ .tick_on = 0, .tick_off = 0, .channel = 0,
                    .note = 64, .velocity = 100, .articulation = 0 });
    vec.push_back({ .tick_on = 0, .tick_off = 0, .channel = 0,
                    .note = 67, .velocity = 100, .articulation = 0 });

    int count = 0;
    for (const auto& ev : vec) {
        EXPECT_GT(ev.note, 0u);
        ++count;
    }
    EXPECT_EQ(count, 3);
}

TEST(ArenaEventVector_Clear, ResetsSize) {
    MidiEventArena arena(8);
    ArenaEventVector vec(arena);
    vec.push_back({});
    vec.push_back({});
    ASSERT_EQ(vec.size(), 2u);
    vec.clear();
    EXPECT_EQ(vec.size(), 0u);
    EXPECT_TRUE(vec.empty());
}

// ─── MidiEventArenaAllocator + std::vector ───────────────────────────────

TEST(MidiEventArenaAllocator_Vector, PushBackAndIterate) {
    MidiEventArena arena(64);
    MidiEventArenaAllocator<MidiEvent> alloc(arena);
    std::vector<MidiEvent, MidiEventArenaAllocator<MidiEvent>> vec(alloc);

    vec.push_back({ .tick_on = 0, .tick_off = 480, .channel = 1,
                    .note = 60, .velocity = 100, .articulation = 0 });
    vec.push_back({ .tick_on = 480, .tick_off = 960, .channel = 1,
                    .note = 64, .velocity = 90, .articulation = 1 });

    ASSERT_EQ(vec.size(), 2u);
    EXPECT_EQ(vec[0].note, 60);
    EXPECT_EQ(vec[1].note, 64);
    EXPECT_EQ(vec[0].tick_on, 0u);
    EXPECT_EQ(vec[1].tick_on, 480u);
}

TEST(MidiEventArenaAllocator_Vector, ThrowsBadAllocWhenFull) {
    MidiEventArena arena(1);
    MidiEventArenaAllocator<MidiEvent> alloc(arena);
    std::vector<MidiEvent, MidiEventArenaAllocator<MidiEvent>> vec(alloc);

    vec.push_back({}); // consumes the single slot
    EXPECT_THROW(vec.push_back({}), std::bad_alloc);
}

TEST(MidiEventArenaAllocator_Vector, MultipleVectorsShareArena) {
    MidiEventArena arena(32);
    MidiEventArenaAllocator<MidiEvent> alloc(arena);

    std::vector<MidiEvent, MidiEventArenaAllocator<MidiEvent>> v1(alloc);
    std::vector<MidiEvent, MidiEventArenaAllocator<MidiEvent>> v2(alloc);

    // Reserve to avoid reallocation surprises from std::vector's growth.
    v1.reserve(2);
    v2.reserve(1);

    v1.push_back({ .tick_on = 0, .note = 60 });
    v1.push_back({ .tick_on = 120, .note = 64 });
    v2.push_back({ .tick_on = 240, .note = 67 });

    EXPECT_EQ(v1.size(), 2u);
    EXPECT_EQ(v2.size(), 1u);
    EXPECT_EQ(v1[0].note, 60);
    EXPECT_EQ(v1[1].note, 64);
    EXPECT_EQ(v2[0].note, 67);
    EXPECT_EQ(arena.used(), 3u);
}

} // namespace
} // namespace aimidi::core
