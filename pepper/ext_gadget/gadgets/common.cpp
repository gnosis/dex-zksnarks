#include "common.hpp"

namespace ethsnarks {

// Copied from `int_list_to_bits`
libff::bit_vector bytes_to_bv(const uint8_t *in_bytes, const size_t in_count)
{
    libff::bit_vector res(in_count * 8);
    for( size_t i = 0; i < in_count; i++ )
    {
        for( size_t j = 0; j < 8; j++ ) {
            res[i * 8 + j] = (in_bytes[i] & (1 << (8 - 1 - j))) ? 1 : 0;
        }
    }
    return res;
}

VariableArrayT VariableArray_from_bits(
    ProtoboardT &in_pb,
    const libff::bit_vector& bits,
    const std::string &annotation_prefix)
{
    VariableArrayT out;
    out.allocate(in_pb, bits.size(), annotation_prefix);
    out.fill_with_bits(in_pb, bits);
    return out;
}

}