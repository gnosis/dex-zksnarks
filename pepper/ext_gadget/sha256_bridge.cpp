#include <iostream>

#include "libsnark/gadgetlib1/gadget.hpp"
#include "libff/common/default_types/ec_pp.hpp"

#include <libsnark/common/default_types/r1cs_ppzksnark_pp.hpp>
#include <libsnark/gadgetlib1/gadgets/hashes/sha256/sha256_gadget.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp>

#include <gadgets/sha256_full.cpp>

using namespace libsnark;
using namespace libff;

typedef libff::Fr<libsnark::default_r1cs_ppzksnark_pp> FieldT;

#define BUFLEN 10240

#define INPUT_SIZE 512
#define OUTPUT_SIZE 256

void constraint_to_json(linear_combination<FieldT> constraints, std::stringstream &ss)
{
    ss << "{";
    uint count = 0;
    for (const linear_term<FieldT>& lt : constraints.terms)
    {
		if (lt.coeff == 0) {
            continue;
        } 
        if (count != 0) {
            ss << ",";
        }
        
        ss << '"' << lt.index << '"' << ":" << '"' << lt.coeff.as_bigint() << '"';
        count++;
    }
    ss << "}";
}

std::string r1cs_to_json(protoboard<FieldT> pb)
{
    r1cs_constraint_system<FieldT> constraints = pb.get_constraint_system();
    std::stringstream ss;
    for (size_t c = 0; c < constraints.num_constraints(); ++c)
    {
        ss << "[";// << "\"A\"=";
        constraint_to_json(constraints.constraints[c].a, ss);
        ss << ",";// << "\"B\"=";
        constraint_to_json(constraints.constraints[c].b, ss);
        ss << ",";// << "\"C\"=";;
        constraint_to_json(constraints.constraints[c].c, ss);
    	ss << "]\n";
    }
    ss.rdbuf()->pubseekpos(0, std::ios_base::out);
    return ss.str();
}

std::string witness_to_json(protoboard<FieldT> pb) {
	r1cs_auxiliary_input<FieldT> values = pb.full_variable_assignment();
	std::stringstream ss;
	for (size_t i = 0; i < values.size(); ++i) {
		ss << values[i].as_bigint() << " ";
	}
	return ss.str();
}

protoboard<FieldT> _getProtoboard(const char* assignment)
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

int main(int argc, char **argv) 
{ 
	if (argc != 2) {
		std::cout << "Needs to be called with a command.\n";
		return 1;
	}

	if (strcmp("size", argv[1]) == 0) {
		protoboard<FieldT> pb = _getProtoboard(nullptr);
		std::cout << (pb.num_variables() - INPUT_SIZE - OUTPUT_SIZE) << "\n";
	} else if (strcmp("constraints", argv[1]) == 0) {
		protoboard<FieldT> pb = _getProtoboard(nullptr);
		std::cout << r1cs_to_json(pb);
	} else if (strcmp("witness", argv[1]) == 0) {
		std::string input_line;
		std::getline(std::cin, input_line);
		protoboard<FieldT> pb = _getProtoboard(input_line.c_str());
		std::cout << witness_to_json(pb);
	} else {
		std::cout << "Unknown command `" << argv[1] << "`! Expecting size, constraints or witness.\n";
		return 1;
	}
    return 0; 
} 