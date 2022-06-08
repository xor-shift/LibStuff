#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <numeric>
#include <span>
#include <string_view>
#include <tuple>
#include <vector>

namespace Stf {

template<typename Char = char>
struct BufferedDelimitedReader {
    size_t match_size = 0;
    std::basic_string_view<Char> delimiter;
    std::vector<Char> buffer{};

    explicit BufferedDelimitedReader(std::basic_string_view<Char> delimiter, size_t capacity = 128)
      : delimiter(delimiter) {
        buffer.reserve(capacity);
    }

    size_t feed(std::basic_string_view<Char> data) {
        auto it = data.begin();
        while (feed_char(*it++));
        --it;
        return std::distance(data.begin(), it);
    }

    /// @return
    /// `true` if the char was consumed\n
    /// `false` indicates that a message has been read or that the buffer has been filled
    bool feed_char(Char c) {
        if (match_size == delimiter.size())
            return false;

        const auto expected_delim_char = delimiter[match_size];
        if (expected_delim_char == c) {
            ++match_size;
            return true;
        }

        const size_t needed_capacity = std::distance(delimiter.begin(), delimiter.end() + match_size) + 1;
        if (needed_capacity > (buffer.capacity() - buffer.size()))
            return false;

        std::copy(delimiter.begin(), delimiter.begin() + match_size, back_inserter(buffer));
        match_size = 0;

        buffer.push_back(c);

        return true;
    }

    bool has_message_ready() const {
        return delimiter.size() == match_size;
    }

    void reset() {
        buffer.clear();
        match_size = 0;
    }

    std::basic_string_view<Char> get_buffered_message() const {
        return {buffer.data(), buffer.size()};
    }

    std::vector<uint8_t>& get_mutable_buffered_message() {
        return buffer;
    }
};

/// COBS encodes some data in-place
/// @param data The data to be transformed using COBS
/// @return
/// Returns the OHB (overhead byte) and the amount of data consumed (might not equal `size`)
extern std::pair<uint8_t, size_t> cobs_encode_inplace(uint8_t* data, size_t size);

template<typename ByteFn, typename BufFn>
struct COBSEncoder {
    ByteFn byte_fn;
    BufFn buf_fn;

    size_t buf_ptr = 0;
    std::array<uint8_t, 255> buffer {};

    void write(uint8_t b) {

        if (buffer.size() == buf_ptr) {
            end_segment_full();
            buf_ptr = 0;
        }

        if (b == 0)
            return end_segment_zero();


    }

    template<typename It>
    requires requires(It a, It b) {
                     { a != b } -> std::convertible_to<bool>;
                     { ++a } -> std::convertible_to<It>;
                     { *a } -> std::convertible_to<uint8_t>;
                 }
    void write(It beg, It end) {
        for (auto it = beg; it != end; ++it)
            write(*it);
    }

    void flush() {
        end_segment_zero();
        byte_fn(0);
    }

private:
    void end_segment_full() {
        byte_fn(0xFF);
        buf_fn(buffer.data(), buffer.size());
        buf_ptr = 0;
    }

    void end_segment_zero() {
        byte_fn(buf_ptr + 1);
        buf_fn(buffer.data(), buffer.size());
        buf_ptr = 0;
    }
};

template<typename ByteFn, typename BufFn>
void cobs_encode(uint8_t* data, size_t size, ByteFn&& byte_fn, BufFn&& buf_fn) {
    while (size != 0) {
        const auto[ohb, round_consumed] = Stf::cobs_encode_inplace(data, size);

        std::invoke(byte_fn, ohb);
        std::invoke(buf_fn, data, round_consumed);

        data += round_consumed;
        size -= round_consumed;

    }
    std::invoke(byte_fn, 0);
}

enum class COBSResult {
    /// Message was properly formatted, data has been consumed fully
    Ok,
    /// Message was one byte short for the ending zero delimiter, data has been consumed fully
    ZeroMissing,
    /// Message had multiple bytes missing up until some indicated zero byte, the data has been consumed
    TooShort,
    /// Parsing halted on an unexpected zero byte, data might not have been consumed fully
    UnexpectedZero,
    /// Internal error, this should never be the case
    Invalid,
};

/// Decodes some COBS-encoded data with a delimiter of '\0'\n
/// \param data
/// \param size
/// \return The size of the decoded data and the decoding result
extern std::pair<size_t, COBSResult> cobs_decode_inplace(uint8_t* data, size_t size);

}

namespace Stf::Delim {

template<size_t BufSize, size_t DelimiterSize, typename Byte = uint8_t>
struct ByteReader {
    static constexpr size_t max_actual_message_size = BufSize - DelimiterSize;
    static constexpr size_t max_message_size = BufSize;
    static constexpr size_t delimiter_size = DelimiterSize;

    std::array<Byte, DelimiterSize> delimiter{};
    std::array<Byte, BufSize> buffer{};
    size_t delim_match_size = 0;
    size_t read_size = 0;

    bool update(Byte b) {
        if (read_size >= BufSize)
            return false;

        if (delim_match_size >= DelimiterSize)
            return false;

        buffer[read_size++] = b;

        if (delimiter[delim_match_size] == b)
            ++delim_match_size;

        return true;
    }

    bool have_match() const {
        return delim_match_size >= delimiter.size();
    }

    bool is_full() const {
        return read_size == BufSize;
    }

    std::span<const Byte> get_with_delimiter() const {
        return {buffer.cbegin(), read_size};
    }

    std::span<const Byte> get_without_delimiter() const {
        return {buffer.cbegin(), read_size - DelimiterSize};
    }

    std::span<Byte> get_with_delimiter() {
        return {buffer.begin(), read_size};
    }

    std::span<Byte> get_without_delimiter() {
        return {buffer.begin(), read_size - DelimiterSize};
    }

    void reset() {
        delim_match_size = 0;
        read_size = 0;
    }

    void reset_safe() {
        reset();
        std::fill(buffer.begin(), buffer.end(), 0);
    }
};

}
