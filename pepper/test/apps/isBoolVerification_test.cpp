#include <cassert>
#include <gtest/gtest.h>
#include "declarations.h"

#define ORDERS 2

struct Private { bool orders[ORDERS][253]; };


#include <apps/dex_common.h>
#include "util.h"

struct ShaHash2 { field digest[256]; };


TEST(isBoolVerification, isBoolVerificationIsBool) { 
    uint32_t length = 2;
    struct ShaHash2 bits[1] = { 0 };
    bits->digest[1] =field("1");
    bits->digest[2] =field("0");

    EXPECT_NO_THROW(isBoolVerification(bits->digest, length));
}

TEST(isBoolVerification, isBoolVerificationNoBool) {
    uint32_t length = 2;
    struct ShaHash2 bits[1]= {0};
    bits->digest[1] =field("2");
    bits->digest[2] =field("0");

    ASSERT_DEATH(isBoolVerification(bits->digest, length), "");
}

int main(int argc, char **argv) {
    testing::FLAGS_gtest_death_test_style = "threadsafe";
    libsnark::default_r1cs_ppzksnark_pp::init_public_params();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}