#pragma once
#include <array>
#include <atomic>
#include <cassert>
#include <type_traits>

// Basic Buffer is designed for 1 producer - 1 consumer
template <typename T, size_t N>
requires std::is_trivial_v<T>
class BasicBuffer {
public:
    // TODO check if memory order makes a difference performance-wise
    bool CanPost() const { return is_full.load(); }
    bool CanReceive() const { return is_full.load(); }

    void ReceiveTo(std::array<T, N>& dest) {
        assert(CanReceive());
        // TODO what will it compile to?
        dest = buf;
        is_full = false;
    }

    void PostFrom(const std::array<T, N>& dest) {
        assert(CanPost());
        buf = dest;
        is_full = true;
    }

private:
    std::atomic_bool is_full = false;
    std::array<T, N> buf;
};

// CyclicBuffer allows one-way communication between 2 threads: producer writes
// data to the end, while consumer reads from the beginning
template <typename T, size_t N>
requires std::is_trivial_v<T>
class CyclicBuffer {
public:
    size_t Size() const { return N; }

    // non-atomic, no sync between threads
    T& operator[](const size_t index) { return buf[index]; }

    // atomic
    size_t GetBeginIndex() const {
        return beginIndex.load(std::memory_order_acquire);
    }
    size_t GetEndIndex() const {  //
        return endIndex.load(std::memory_order_acquire);
    }
    void UpdateBeginIndex(const size_t val) {
        beginIndex.store(val, std::memory_order_release);
    }
    void UpdateEndIndex(const size_t val) {
        endIndex.store(val, std::memory_order_release);
    }

private:
    std::atomic_size_t beginIndex = 0;
    std::atomic_size_t endIndex = 0;
    T buf[N];
};
