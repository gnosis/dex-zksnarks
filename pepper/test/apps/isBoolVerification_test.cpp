#include <cassert>
#include <gtest/gtest.h>
#include <algorithm>
#include "declarations.h"

struct Private {};

#include <apps/hashing.h>
#include "util.h"


TEST(isBoolVerification, isBoolVerificationWithBools) { 
    uint32_t length = 2;
    field254 bits[2] = { 0 };
    bits[0] = field254("1");
    bits[1] = field254("0");
    EXPECT_NO_THROW(isBoolVerification(bits, length));
}

TEST(isBoolVerification, isBoolVerificationWithNonbools) {
    uint32_t length = 2;
    field254 bits[2] = { 0 };
    bits[0] = field254("2");
    bits[1] = field254("0");

    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(isBoolVerification(bits, length), "");
    DISABLE_STACKTRACE = false;
}

int main(int argc, char **argv) {
    testing::FLAGS_gtest_death_test_style = "threadsafe";
    libsnark::default_r1cs_ppzksnark_pp::init_public_params();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}