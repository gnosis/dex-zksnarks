#include "common.h"
#include <gadgets/sha256_full.cpp>

using namespace libsnark;
using namespace libff;

#define BUFLEN 10240
#define INPUT_SIZE 512
#define OUTPUT_SIZE 256

uint64_t inputSize() {
	return INPUT_SIZE;
}

uint64_t outputSize() {
	return OUTPUT_SIZE;
}

protoboard<FieldT> getProtoboard(const char* assignment)
{
    libsnark::default_r1cs_ppzksnark_pp::init_public_params();
    protoboard<FieldT> pb;

    block_variable<FieldT> input(pb, INPUT_SIZE, "input");
    digest_variable<FieldT> output(pb, OUTPUT_SIZE, "output");

    ethsnarks::sha256_full_gadget_512 f(pb, input, output, "f");
    f.generate_r1cs_constraints();

	if (assignment) {
		libff::bit_vector input_bv;
     	for (int i = 0; i < INPUT_SIZE; i++) {
        	input_bv.push_back(assignment[2*i] - '0');
    	}
     	input.generate_r1cs_witness(input_bv);
     	f.generate_r1cs_witness();
    
    	assert(pb.is_satisfied());
	}
    
    return pb;
}