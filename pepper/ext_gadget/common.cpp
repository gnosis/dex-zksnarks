#include <iostream>

#include "common.h"

using namespace libsnark;

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

int main(int argc, char **argv) 
{ 
	if (argc != 2) {
		std::cout << "Needs to be called with a command.\n";
		return 1;
	}

	if (strcmp("size", argv[1]) == 0) {
		protoboard<FieldT> pb = getProtoboard(nullptr);
		std::cout << (pb.num_variables() - inputSize() - outputSize()) << "\n";
	} else if (strcmp("constraints", argv[1]) == 0) {
		protoboard<FieldT> pb = getProtoboard(nullptr);
		std::cout << r1cs_to_json(pb);
	} else if (strcmp("witness", argv[1]) == 0) {
		std::string input_line;
		std::getline(std::cin, input_line);
		protoboard<FieldT> pb = getProtoboard(input_line.c_str());
		std::cout << witness_to_json(pb);
	} else {
		std::cout << "Unknown command `" << argv[1] << "`! Expecting size, constraints or witness.\n";
		return 1;
	}
    return 0; 
} 