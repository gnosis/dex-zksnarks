#include <stdint.h>
#include <libsnark/common/default_types/r1cs_ppzksnark_pp.hpp>

typedef libff::alt_bn128_pp ppT;
typedef libff::Fr<ppT> field;


//Declaration of internal pepper methods (to make compiler happy)
void exo_compute(field**, uint32_t*, void*, uint32_t);
void ext_gadget(void*, void*, uint32_t);
void assert_zero(field);
