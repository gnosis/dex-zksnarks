#include <cassert>
#include <gtest/gtest.h>

#include <apps/dex_common.h>
#include "util.h"


//#ifndef ALT_BN128_INIT_HPP_
//#define ALT_BN128_INIT_HPP_
#include <libff/algebra/fields/fp.hpp>
//#include <libff>
#include <libff/algebra/curves/public_params.hpp>

const mp_size_t alt_bn128_r_bitcount = 254;
const mp_size_t alt_bn128_r_limbs = (alt_bn128_r_bitcount+GMP_NUMB_BITS-1)/GMP_NUMB_BITS;


// typedef Fp_model<n, modulus> my_Fp;
//typedef Fp_model<alt_bn128_r_limbs, 21888242871839275222246405745257275088548364400416034343698204186575808495617> my_Fp;
typedef Fp_model<alt_bn128_r_limbs, 2188824287183927> my_Fp;


TEST(isBoolVerification, isBoolVerificationIsBool) { 
    my_Fp bits = 434;
    printf(bits)
    EXPECT_ANY_THROW(isBoolVerification(&bits, length));
}

TEST(isBoolVerification, isBoolVerificationNoBool) {
    bool bits = {1, 0, 12};
    uint32_t length = 3;
    ASSERT_DEATH(isBoolVerification(&bits, length), "");
}
 
int main(int argc, char **argv) {
    testing::FLAGS_gtest_death_test_style = "threadsafe";
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}