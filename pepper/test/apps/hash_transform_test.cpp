#include <cassert>
#include <gtest/gtest.h>

#define ORDERS 2

#include <apps/hash_transform.c>
#include "util.h"

TEST(HashTransformTest, TransformWithMatchingSHA) { 
    pepper_overrides::shaCount = 0;
    struct In input = { 0,3 }; /* resulting SHA after two rounds will be 11 */
    struct Out output;
    compute(&input, &output);
    ASSERT_EQ(output.pedersenHash[0], 2);
}

TEST(HashTransformTest, TransformWithWrongSHA) {
    struct In input = { -1 };
    struct Out output;
    ASSERT_DEATH(compute(&input, &output), "");
}
 
int main(int argc, char **argv) {
    testing::FLAGS_gtest_death_test_style = "threadsafe";
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}