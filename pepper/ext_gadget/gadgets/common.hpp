#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>
#include <libsnark/gadgetlib1/protoboard.hpp>
#include <libsnark/gadgetlib1/gadget.hpp>

namespace ethsnarks {

typedef libff::bigint<libff::alt_bn128_r_limbs> LimbT;
typedef libff::alt_bn128_G1 G1T;
typedef libff::alt_bn128_G2 G2T;
typedef libff::alt_bn128_pp ppT;
typedef libff::Fq<ppT> FqT;
typedef libff::Fr<ppT> FieldT;
typedef libsnark::r1cs_constraint<FieldT> ConstraintT;
typedef libsnark::protoboard<FieldT> ProtoboardT;
typedef libsnark::pb_variable<ethsnarks::FieldT> VariableT;
typedef libsnark::pb_variable_array<FieldT> VariableArrayT;
typedef libsnark::gadget<ethsnarks::FieldT> GadgetT;

// Copied from `int_list_to_bits`
libff::bit_vector bytes_to_bv(const uint8_t *in_bytes, const size_t in_count);

// Convert a bit vector to a pb_variable_array
VariableArrayT VariableArray_from_bits( ProtoboardT &in_pb, const libff::bit_vector& bits, const std::string &annotation_prefix="" );

}

#endif
