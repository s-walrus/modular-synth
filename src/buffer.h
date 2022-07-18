#pragma once
#include <array>
#include <atomic>
#include <cassert>
#include <type_traits>

// Basic Buffer is designed for 1 producer - N consumers
// (there definitely is a room for improvement in performance)
template <typename T, size_t N>
requires std::is_trivial_v<T>
class BasicBuffer {
public:
    using DataGeneration = std::uint16_t;

    BasicBuffer(size_t n_consumers)
        : n_consumers(n_consumers), receiver_cnt(n_consumers) {}

    bool CanPost() const { return receiver_cnt.load() == n_consumers; }
    bool CanReceive(std::size_t last_received_generation) const {
        // FIXME this assert must hold true but it doesn't (in gdb it does
        // suprisingly, maybe, it's a data-race)
        // assert(last_received_generation == generation.load() || !CanPost());
        return last_received_generation != generation.load();
    }

    DataGeneration ReceiveTo(std::array<T, N>& dest) {
        // TODO what will it compile to?
        dest = buf;
        receiver_cnt.fetch_add(1);
        return generation.load();
    }

    DataGeneration PostFrom(const std::array<T, N>& dest) {
        assert(CanPost());
        buf = dest;
        generation.fetch_add(1, std::memory_order_seq_cst);
        // WARNING If ReceiveTo is called between these instructions,
        // n_consumers temporarily stop reflecting real consumer count for
        // current generation
        receiver_cnt.fetch_sub(n_consumers, std::memory_order_seq_cst);
        return generation.load();
    }

private:
    // TODO support thread-safe n_consumers modification
    const std::size_t n_consumers;
    std::atomic<DataGeneration> generation = 0;
    std::atomic_size_t receiver_cnt;
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
