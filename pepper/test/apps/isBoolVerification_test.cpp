#include <cassert>
#include <gtest/gtest.h>
#include "declarations.h"

#define ORDERS 2

struct Private { bool orders[ORDERS][253]; };


#include <apps/dex_common.h>
#include "util.h"



TEST(isBoolVerification, isBoolVerificationIsBool) { 
    my_Fp *bits;
    bits[0] =libff::Fp_model<alt_bn128_r_limbs, pf>("0");
    bits[1] =libff::Fp_model<alt_bn128_r_limbs, pf>("1");

    //printf(bits)
    uint32_t length = 2;
    //EXPECT_ANY_THROW(isBoolVerification(bits, length));
}

TEST(isBoolVerification, isBoolVerificationNoBool) {
    my_Fp *bits;
    bits[0] =libff::Fp_model<alt_bn128_r_limbs, pf>("0");
    bits[1] =libff::Fp_model<alt_bn128_r_limbs, pf>("1");

    //printf(bits)
    uint32_t length = 2;
    ASSERT_DEATH(isBoolVerification(bits, length), "");
}
 
int main(int argc, char **argv) {
    testing::FLAGS_gtest_death_test_style = "threadsafe";
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}