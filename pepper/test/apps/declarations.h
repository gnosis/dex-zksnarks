#include <stdint.h>
//Declaration of internal pepper methods (to make compiler happy)



//#ifndef ALT_BN128_INIT_HPP_
//#define ALT_BN128_INIT_HPP_
#include <libff/algebra/fields/fp.hpp>
//#include <libff/algebra/curves/public_params.hpp>

const mp_size_t alt_bn128_r_bitcount = 254;
const mp_size_t alt_bn128_r_limbs = (alt_bn128_r_bitcount+GMP_NUMB_BITS-1)/GMP_NUMB_BITS;


// typedef Fp_model<n, modulus> my_Fp;
//typedef Fp_model<alt_bn128_r_limbs, 21888242871839275222246405745257275088548364400416034343698204186575808495617> my_Fp;
libff::bigint<alt_bn128_r_limbs> pf =libff::bigint<alt_bn128_r_limbs>("21888242871839275222246405745257275088548364400416034343698204186575808495617");
typedef libff::Fp_model<alt_bn128_r_limbs, pf> my_Fp;

void exo_compute(uint32_t**, uint32_t*, void*, uint32_t);
void ext_gadget(void*, void*, uint32_t);
void assert_zero(my_Fp input){
	my_Fp zero =libff::Fp_model<alt_bn128_r_limbs, pf>("0");
	if(zero == input){
		        throw std::invalid_argument( "received non-zero field element" );
	}
}
typedef long int128;
