#include "common.h"
#include "gadgets/jubjub/pedersen_commitment.hpp"

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
    libsnark::default_r1cs_ppzksnark_pp::init_public_params();
    protoboard<FieldT> pb;

	pb_variable_array<FieldT> left;
    left.allocate(pb, LEFT_INPUT_SIZE, FMT("annotation_prefix", " scaler to multiply by"));
    
	pb_variable_array<FieldT> right;
    right.allocate(pb, RIGHT_INPUT_SIZE, FMT("annotation_prefix", " scaler to multiply by"));

	pb_variable<FieldT> commitment_x;
    pb_variable<FieldT> commitment_y;
    commitment_x.allocate(pb, "r_x");
    commitment_y.allocate(pb, "r_y");

    ethsnarks::pedersen_commitment f(pb,left,right,commitment_x,commitment_y);
    f.generate_r1cs_constraints();

	if (assignment) {
		libff::bit_vector left_bv;
		libff::bit_vector right_bv;
     	for (int i = 0; i < LEFT_INPUT_SIZE; i++) {
        	left_bv.push_back(assignment[2*i] - '0');
    	}
		left.fill_with_bits(pb, left_bv);

		for (int i = LEFT_INPUT_SIZE; i < LEFT_INPUT_SIZE + RIGHT_INPUT_SIZE; i++) {
        	right_bv.push_back(assignment[2*i] - '0');
    	}
		right.fill_with_bits(pb, right_bv);
     	f.generate_r1cs_witness();
		assert(pb.is_satisfied());
	}
    
    return pb;
}