#pragma once

#include <cstdint>

#include <Stuff/Refl/ReflNew.hpp>
#include <Stuff/Refl/Serde.hpp>
#include <Stuff/Maths/Check/CRC.hpp>

#include "./Delim.hpp"

namespace Stf {

struct PacketHeader {
    MEMREFL_BEGIN(PacketHeader, 4)

    uint32_t MEMREFL_DECL_MEMBER(crc);
    uint16_t MEMREFL_DECL_MEMBER(len);
    uint16_t MEMREFL_DECL_MEMBER(id);
    uint32_t MEMREFL_DECL_MEMBER(order);
};

struct PacketManStatistics {
    size_t current_overruns = 0;
    size_t total_overruns = 0;
    size_t decode_errors = 0;
    size_t crc_errors = 0;
    size_t length_errors = 0;
    size_t rx_redundant_packets = 0;
    size_t rx_dropped_packets = 0;
    size_t rx_packets = 0;

    size_t state_resets = 0;

    void reset_state() {
        size_t next_state_resets = state_resets + 1;

        *this = {};

        state_resets = next_state_resets;
    }
};

template<size_t TxBufSize, size_t RxBufSize>
// requires(typename std::invoke_result_t<SendCallback, const uint8_t*, size_t>)
struct PacketManagerBase {
    struct PingPacket {
        static constexpr uint16_t packet_id = 0xFFFF;

        MEMREFL_BEGIN(PingPacket, 1);

        uint32_t token;

        MEMREFL_MEMBER(token);
    };

    size_t initial_drop_size = 3;
    size_t remaining_drops = initial_drop_size;
    std::array<uint8_t, TxBufSize> tx_buf {};
    Stf::Delim::ByteReader<RxBufSize, 1> rx_reader {
        .delimiter = { 0 },
    };

    uint32_t last_sent_packet_order = 0;
    uint32_t last_received_packet_order = 0;

    PacketManStatistics stats {};

    virtual void tx_buffer(uint8_t* b, size_t n) { }

    virtual void rx_packet(std::span<const uint8_t> p, PacketHeader header) { }

    virtual void prepare_tx() {}

    template<typename T> uint32_t tx_packet(T const& v, size_t count = 1) {
        if (!serialize_data(v))
            return 0;

        static constexpr size_t total_size = serialized_size_v<T> + serialized_size_v<PacketHeader>;

        const size_t encoded_size = encode_data<T>();
        if (encoded_size == 0)
            return 0;

        ++last_sent_packet_order;

        while (count-- > 0) {
            prepare_tx();
            tx_buffer(tx_buf.data(), encoded_size);
        }
        return last_sent_packet_order;
    }

    void rx_byte(uint8_t b) {
        /*if (remaining_drops != 0) {
            --remaining_drops;
            return;
        }*/

        if (rx_reader.update(b))
            return;

        finish_rx();

        stats.total_overruns += stats.current_overruns;
        rx_reader.reset();
        rx_reader.update(b);
    }

    void reset() {
        stats.reset_state();
        last_sent_packet_order = 0;
        last_received_packet_order = 0;
    }

private:
    /// Serializes some data to tx_buf\n
    /// The data will be right-aligned\n
    /// @return
    /// If false, the serialized data isn't going to fit the transmission buffer
    template<typename T> bool serialize_data(T const& v) {
        static constexpr size_t total_needed_size = serialized_size_v<PacketHeader> + serialized_size_v<T>;
        if constexpr (total_needed_size > TxBufSize)
            return false;
        static constexpr size_t leading_free_space = TxBufSize - total_needed_size;

        PacketHeader header {
            .crc = 0x00000000,
            .len = static_cast<uint16_t>(serialized_size_v<T>),
            .id = T::packet_id,
            .order = last_sent_packet_order + 1,
        };

        serialize(tx_buf.end() - serialized_size_v<T>, v);
        serialize(tx_buf.end() - total_needed_size, header);

        CRCState<CRCDescriptions::CRC32ISOHDLC> context {};
        for (size_t i = 0; i < total_needed_size; i++)
            context.update(tx_buf[i + leading_free_space]);

        header.crc = context.finished_value();
        serialize(tx_buf.end() - total_needed_size, header);

        return true;
    }

    /// COBS-encodes the existing serialized data, in-place within the tx buffer
    /// @tparam T This parameter is used to determine the serialized size of the existing data
    /// @return
    /// 0 if encoding failed, the size of the encoded, left aligned, data if successful
    template<typename T> size_t encode_data() {
        static constexpr size_t total_size = serialized_size_v<T> + serialized_size_v<PacketHeader>;

        size_t read_head = TxBufSize - total_size;
        size_t write_head = 0;
        bool failed = false;

        auto write_byte = [&](uint8_t b) {
            if (failed)
                return;

            if (write_head + 1 >= read_head) {
                failed = true;
                return;
            }

            tx_buf[write_head++] = b;
        };

        auto write_buf = [&](const uint8_t* b, size_t n) {
            if (failed)
                return;

            if (write_head + n >= TxBufSize) {
                failed = true;
                return;
            }

            std::copy(b, b + n, tx_buf.data() + write_head);

            write_head += n;
            read_head += n;
        };

        cobs_encode(tx_buf.data() + read_head, total_size, write_byte, write_buf);

        return failed ? 0 : write_head;
    }

    void finish_rx() {
        if (!rx_reader.have_match()) {
            ++stats.current_overruns;
            return;
        }

        const auto rx_span = rx_reader.get_with_delimiter();

        const auto [decode_size, decode_result] = cobs_decode_inplace(rx_span.data(), rx_span.size());

        if (decode_result != COBSResult::Ok) {
            ++stats.decode_errors;
            return;
        }

        const auto decoded_span = rx_span.subspan(0, decode_size);

        if (decode_size < serialized_size_v<PacketHeader>) {
            ++stats.length_errors;
            return;
        }

        PacketHeader given_header {};
        auto header_end_it = deserialize(given_header, decoded_span.begin());
        const std::span<const uint8_t> payload_span(header_end_it, decoded_span.end());

        std::fill(decoded_span.begin(), decoded_span.begin() + 4, 0);

        CRCState<CRCDescriptions::CRC32ISOHDLC> context {};
        for (auto b : decoded_span)
            context.update(b);

        PacketHeader calculated_header { .crc = context.finished_value(),
            .len = static_cast<uint16_t>(payload_span.size()),
            .id = given_header.id,
            .order = given_header.order };

        if (calculated_header.crc != given_header.crc) {
            ++stats.crc_errors;
            return;
        }

        if (calculated_header.len != given_header.len) {
            ++stats.length_errors;
            return;
        }

        if (given_header.order == last_received_packet_order) {
            ++stats.rx_redundant_packets;
            return;
        } else if (given_header.order < last_received_packet_order) {
            reset();
        }

        stats.rx_dropped_packets += (given_header.order - 1) - last_received_packet_order;
        last_received_packet_order = given_header.order;

        ++stats.rx_packets;
        rx_packet(payload_span, given_header);
    }
};

}
