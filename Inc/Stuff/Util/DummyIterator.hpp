#pragma once

#include <cstddef>
#include <compare>

namespace Stf {

template<typename T>
struct DummyIterator {
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = T const&;

    T dummy_value{};
    difference_type offset = 0;

    constexpr DummyIterator& operator++() {
        offset++;
        return *this;
    }

    constexpr DummyIterator operator++(int) {
        DummyIterator ret = *this;
        ++offset;
        return ret;
    }

    constexpr DummyIterator& operator--() {
        offset--;
        return *this;
    }

    constexpr DummyIterator operator--(int) {
        DummyIterator ret = *this;
        --offset;
        return ret;
    }

    constexpr DummyIterator& operator+=(difference_type n) {
        offset += n;
        return *this;
    }

    friend constexpr DummyIterator operator+(DummyIterator it, difference_type n) {
        return DummyIterator{
            .offset = it.offset + n,
        };
    }

    friend constexpr DummyIterator operator+(difference_type n, DummyIterator it) {
        return DummyIterator{
            .offset = it.offset + n,
        };
    }

    constexpr DummyIterator& operator-=(difference_type n) {
        offset -= n;
        return *this;
    }

    friend constexpr DummyIterator operator-(DummyIterator it, difference_type n) {
        return DummyIterator{
            .offset = it.offset - n,
        };
    }

    friend constexpr difference_type operator-(DummyIterator a, DummyIterator b) {
        return a.offset - b.offset;
    }

    constexpr reference operator[](difference_type) {
        return dummy_value;
    }

    constexpr reference operator*() {
        return (*this)[0];
    }

    constexpr const_reference operator[](difference_type) const {
        return dummy_value;
    }

    constexpr const_reference operator*() const {
        return *this[0];
    }

    friend constexpr std::strong_ordering operator<=>(DummyIterator a, DummyIterator b) {
        return a.offset <=> b.offset;
    }
};

}
