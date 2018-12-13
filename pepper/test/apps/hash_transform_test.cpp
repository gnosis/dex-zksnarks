#include <cassert>
#include <gtest/gtest.h>
#include "declarations.h"

#define ORDERS 2

#include <apps/hash_transform.c>
#include "util.h"

TEST(HashTransformTest, TransformWithMatchingSHA) { 
    privateInput = { {0} };
    ShaResult shaHash = hashSHA(privateInput.orders, 0, ORDERS*253, 253);
    field254 pedersenHash = hashPedersen(privateInput.orders, 0, ORDERS*253, 253);

    struct In input = { shaHash.left, shaHash.right };
    struct Out output;
    compute(&input, &output);
    ASSERT_EQ(output.pedersenHash, pedersenHash);
}

TEST(HashTransformTest, TransformWithWrongSHA) {
    struct In input = { 5 };
    struct Out output;

    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}
 
int main(int argc, char **argv) {
    testing::FLAGS_gtest_death_test_style = "threadsafe";
    libsnark::default_r1cs_ppzksnark_pp::init_public_params();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}