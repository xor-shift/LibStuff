#include <Stuff/IO/Delim.hpp>

namespace Stf {

extern std::pair<uint8_t, size_t> cobs_encode_inplace(uint8_t* data, size_t size) {
    if (size == 0)
        return {0x01, 0};

    std::pair<uint8_t, size_t> ret{0, 0};
    auto&[beg_byte, consumed] = ret;

    auto* const end = data + size;
    auto* it = data;

    for (auto* prev_zero_pt = &beg_byte;; it++) {
        const auto dist = prev_zero_pt == &beg_byte ? (1 + std::distance(data, it)) : std::distance(prev_zero_pt, it);

        if (it == end || dist == 255) {
            *prev_zero_pt = dist;
            break;
        }

        if (*it == 0) {
            *prev_zero_pt = dist;
            prev_zero_pt = it;
        }
    }

    consumed = std::distance(data, it);

    return ret;
}

struct COBSDecodeState {
    uint8_t* data;
    uint8_t* end;

    uint8_t* current_input_it;
    uint8_t* current_output_it;
};

inline std::pair<COBSDecodeState, COBSResult> cobs_decode_block(COBSDecodeState state) {
    if (state.current_input_it == state.end)
        return {state, COBSResult::Invalid};

    auto remaining_until_zero = *state.current_input_it++;
    auto small_block = remaining_until_zero != 255;
    if (remaining_until_zero == 0)
        return {state, COBSResult::UnexpectedZero};

    for (auto*& it = state.current_input_it;; it++) {
        if (--remaining_until_zero == 0) {
            if (it == state.end) {
                if (small_block)
                    return {state, COBSResult::ZeroMissing};
                else
                    return {state, COBSResult::Ok};
            }

            if (small_block) {
                //if small block, then either: emit zero and continue, find zero and end block
                if (*it == 0) {
                    ++it;
                    return {state, COBSResult::Ok};
                }

                *state.current_output_it++ = 0;
                remaining_until_zero = *it;
                small_block = remaining_until_zero != 255;
            } else { //large block
                if (*it == 0)
                    ++it;
                return {state, COBSResult::Ok};
            }
        } else {
            if (it == state.end) //we should have data remaining, yet we are at the end of the data
                return {state, COBSResult::TooShort};

            if (*it == 0)
                return {state, COBSResult::UnexpectedZero};

            *state.current_output_it++ = *it;
        }
    }
}

extern std::pair<size_t, COBSResult> cobs_decode_inplace(uint8_t* data, size_t size) {
    COBSDecodeState state{
      .data = data,
      .end = data + size,
      .current_input_it = data,
      .current_output_it = data,
    };

    for (;;) {
        auto[new_state, result] = cobs_decode_block(state);
        bool do_exit = false;

        state = new_state;

        switch (result) {
            case COBSResult::Ok:
            default:
                break;
            case COBSResult::Invalid:
                result = COBSResult::Ok;
            case COBSResult::ZeroMissing:
            case COBSResult::TooShort:
            case COBSResult::UnexpectedZero:
                do_exit = true;
                break;
        }

        if (do_exit)
            return {std::distance(state.data, state.current_output_it), result};
    }
}

}
