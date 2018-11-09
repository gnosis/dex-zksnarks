#include "libff/common/default_types/ec_pp.hpp"
#include "libsnark/gadgetlib1/gadget.hpp"

#include <libsnark/common/default_types/r1cs_ppzksnark_pp.hpp>

typedef libff::Fr<libsnark::default_r1cs_ppzksnark_pp> FieldT;

/**
 * Needs to be overridden by concrete bridge and provide a protoboard 
 * given the optional assignment.
 */
libsnark::protoboard<FieldT> getProtoboard(const char* assignment);

/**
 * Needs to be overridden and return the total number of input variables.
 */
uint64_t inputSize();

/**
 * Needs to be overridden and return the total number of output variables.
 */
uint64_t outputSize();