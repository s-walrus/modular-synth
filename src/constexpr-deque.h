#pragma once
#include <cinttypes>
#include <type_traits>

template <typename T, std::size_t capacity>
requires std::is_trivial_v<T>
class Deque {
public:
    constexpr Deque() = default;

    constexpr std::size_t Size() const { return size; }
    constexpr bool Empty() const { return size == 0; }

    constexpr void PushBack(const T& val) {
        content[end] = val;
        end = IncIndex(end);
        ++size;
    }

    constexpr void PushFront(const T& val) {
        begin = DecIndex(begin);
        content[begin] = val;
        ++size;
    }

    constexpr void PopBack() {
        end = DecIndex(end);
        --size;
    }

    constexpr void PopFront() {
        begin = IncIndex(begin);
        --size;
    }

    constexpr T Back() const { return content[DecIndex(end)]; }
    constexpr T Front() const { return content[begin]; }

private:
    T content[capacity];
    std::size_t begin = 0;  // front
    std::size_t end = 0;    // back
    std::size_t size = 0;

    static constexpr std::size_t IncIndex(std::size_t i) {
        if (i == capacity - 1) {
            return 0;
        } else {
            return i + 1;
        }
    }

    static constexpr std::size_t DecIndex(std::size_t i) {
        if (i == 0) {
            return capacity - 1;
        } else {
            return i - 1;
        }
    }
};
