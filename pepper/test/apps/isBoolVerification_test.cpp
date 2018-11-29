#include <cassert>
#include <gtest/gtest.h>
#include "declarations.h"
#include <apps/hash_transform.h>

#include <apps/dex_common.h>
#include "util.h"

TEST(isBoolVerification, isBoolVerificationWithBools) { 
    field254 bits[1] = { 0 };
    bits = field("2");
    bits = field("0");
    EXPECT_NO_THROW(isBoolVerification(bits, length));
}

TEST(isBoolVerification, isBoolVerificationWithNonbools) {
    field254 bits[1] = { 0 };
    bits = field("2");
    bits = field("0");
    ASSERT_DEATH(isBoolVerification(bits, length), "");
}

int main(int argc, char **argv) {
    testing::FLAGS_gtest_death_test_style = "threadsafe";
    libsnark::default_r1cs_ppzksnark_pp::init_public_params();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}