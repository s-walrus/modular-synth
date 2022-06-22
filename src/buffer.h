#include <atomic>
#include <type_traits>

// CyclicBuffer allows one-way communication between 2 threads: producer writes
// data to the end, while consumer reads from the beginning
template <typename T, size_t N>
requires std::is_trivial_v<T>
class CyclicBuffer {
public:
    size_t Size() const { return N; }

    // non-atomic, no sync between threads
    T& operator[](size_t index) { return buf[index]; }

    // atomic
    size_t GetBeginIndex() const {
        return beginIndex.load(std::memory_order_acquire);
    }
    size_t GetEndIndex() const {  //
        return endIndex.load(std::memory_order_acquire);
    }
    void UpdateBeginIndex(size_t val) {
        beginIndex.store(val, std::memory_order_release);
    }
    void UpdateEndIndex(size_t val) {
        endIndex.store(val, std::memory_order_release);
    }

private:
    std::atomic_size_t beginIndex = 0;
    std::atomic_size_t endIndex = 0;
    T buf[N];
};
