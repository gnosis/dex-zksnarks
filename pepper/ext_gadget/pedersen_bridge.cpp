#include "common.h"
#include "jubjub/pedersen_hash.hpp"
#include "jubjub/params.hpp"

using namespace libsnark;
using namespace libff;

#define BUFLEN 10240

#define LEFT_INPUT_SIZE 254
#define RIGHT_INPUT_SIZE 254
#define OUTPUT_SIZE 2

uint64_t inputSize() {
    return LEFT_INPUT_SIZE + RIGHT_INPUT_SIZE;
}

uint64_t outputSize() {
    return OUTPUT_SIZE;
}

protoboard<FieldT> getProtoboard(const char* assignment)
{
    ethsnarks::ppT::init_public_params();
    const ethsnarks::jubjub::Params params;
    protoboard<FieldT> pb;

    ethsnarks::VariableArrayT in;
    // The gadget expects input that is a multiple of three. Thus, we pad with 0s.
    size_t padding  = (3 - (inputSize() % 3)) % 3;
    size_t scalar_size = inputSize() + padding;
    in.allocate(pb, scalar_size, FMT("annotation_prefix", " scaler to multiply by"));

    ethsnarks::jubjub::PedersenHash f(pb, params, "dex.pedersen-hash", in, "gadget");
    f.generate_r1cs_constraints();

    if (assignment) {
	libff::bit_vector in_bv;
     	for (int i = 0; i < scalar_size; i++) {
            if (i < inputSize()) {
                in_bv.push_back(assignment[2*i] - '0');
            } else {
                in_bv.push_back(false);
            }
    	}

	in.fill_with_bits(pb, in_bv);
     	f.generate_r1cs_witness();
	assert(pb.is_satisfied());
    }
    
    return pb;
}
