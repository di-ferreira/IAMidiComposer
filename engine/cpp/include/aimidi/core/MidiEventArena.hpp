// MidiEventArena — pool allocator for MidiEvent vectors used in MTE hot paths.
//
// Pre-allocates a large contiguous block and sub-allocates O(1) without
// individual free. The entire arena is reset at once via reset().
//
// Thread-safety: NOT thread-safe. Each thread (or hot-path context) should
// have its own arena.
//
// Usage:
//   MidiEventArena arena(4096);
//   ArenaEventVector events(arena);
//   events.push_back({...});
//   arena.reset();
#pragma once

#include <aimidi/theory/Types.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <vector>

namespace aimidi::core {

using aimidi::theory::MidiEvent;

class MidiEventArena {
public:
    // Capacity is the number of MidiEvent slots to pre-allocate.
    // Default: 4096 events ≈ 64 KB (4096 * 16 bytes)
    explicit MidiEventArena(std::size_t capacity = 4096)
        : capacity_(capacity)
        , buffer_(std::make_unique<MidiEvent[]>(capacity))
        , used_(0)
    {
    }

    MidiEventArena(const MidiEventArena&) = delete;
    MidiEventArena& operator=(const MidiEventArena&) = delete;
    MidiEventArena(MidiEventArena&&) noexcept = default;
    MidiEventArena& operator=(MidiEventArena&&) noexcept = default;

    // Reset the arena: all previously allocated memory becomes available again.
    // O(1) — does not call destructors on MidiEvent (trivially destructible).
    void reset() noexcept { used_ = 0; }

    // Allocate a single MidiEvent from the arena.
    // Returns nullptr if capacity is exhausted.
    MidiEvent* allocate() noexcept {
        if (used_ >= capacity_)
            return nullptr;
        return &buffer_[used_++];
    }

    // Allocate space for n MidiEvents.
    // Returns nullptr if insufficient capacity.
    MidiEvent* allocate_n(std::size_t n) noexcept {
        if (used_ + n > capacity_)
            return nullptr;
        auto* ptr = &buffer_[used_];
        used_ += n;
        return ptr;
    }

    std::size_t used() const noexcept { return used_; }
    std::size_t capacity() const noexcept { return capacity_; }
    std::size_t remaining() const noexcept { return capacity_ - used_; }

    MidiEvent*       data() noexcept { return buffer_.get(); }
    const MidiEvent* data() const noexcept { return buffer_.get(); }

private:
    std::size_t                 capacity_;
    std::unique_ptr<MidiEvent[]> buffer_;
    std::size_t                 used_;
};

// A convenience wrapper that provides a std::vector-like interface
// backed by MidiEventArena memory.
//
// This is NOT a full std::vector replacement — it only supports push_back
// and iteration. For full std::vector functionality, use the arena with
// a custom allocator (see MidiEventArenaAllocator below).
class ArenaEventVector {
public:
    using iterator       = MidiEvent*;
    using const_iterator = const MidiEvent*;

    explicit ArenaEventVector(MidiEventArena& arena)
        : arena_(&arena)
        , data_(nullptr)
        , size_(0)
    {
    }

    // Push a copy of event into arena memory.
    // Returns false if arena is full.
    bool push_back(const MidiEvent& event) noexcept {
        auto* slot = arena_->allocate();
        if (!slot)
            return false;
        *slot = event;
        if (size_ == 0)
            data_ = slot;
        ++size_;
        return true;
    }

    MidiEvent&       operator[](std::size_t i) noexcept {
        assert(i < size_);
        return data_[i];
    }
    const MidiEvent& operator[](std::size_t i) const noexcept {
        assert(i < size_);
        return data_[i];
    }

    std::size_t size() const noexcept { return size_; }
    bool        empty() const noexcept { return size_ == 0; }

    iterator       begin() noexcept { return data_; }
    iterator       end() noexcept { return data_ + size_; }
    const_iterator begin() const noexcept { return data_; }
    const_iterator end() const noexcept { return data_ + size_; }

    void clear() noexcept { size_ = 0; }

private:
    MidiEventArena* arena_;
    MidiEvent*      data_;
    std::size_t     size_;
};

// STL-compatible allocator using MidiEventArena.
// Allows std::vector<MidiEvent, MidiEventArenaAllocator> to use arena memory.
template <typename T>
class MidiEventArenaAllocator {
public:
    using value_type = T;

    explicit MidiEventArenaAllocator(MidiEventArena& arena) noexcept
        : arena_(&arena)
    {
    }

    template <typename U>
    MidiEventArenaAllocator(const MidiEventArenaAllocator<U>& other) noexcept
        : arena_(other.arena_)
    {
    }

    T* allocate(std::size_t n) {
        auto* ptr = reinterpret_cast<T*>(
            arena_->allocate_n(n * sizeof(T) / sizeof(MidiEvent)));
        if (!ptr)
            throw std::bad_alloc();
        return ptr;
    }

    void deallocate(T*, std::size_t) noexcept {
        // Arena allocator does not free individual allocations.
    }

    template <typename U>
    bool operator==(const MidiEventArenaAllocator<U>& other) const noexcept {
        return arena_ == other.arena_;
    }

    template <typename U>
    bool operator!=(const MidiEventArenaAllocator<U>& other) const noexcept {
        return arena_ != other.arena_;
    }

    MidiEventArena* arena_;
};

} // namespace aimidi::core
