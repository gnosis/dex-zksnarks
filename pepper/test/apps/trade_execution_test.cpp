#include <cassert>
#include <gtest/gtest.h>
#include "declarations.h"

#define ORDERS 2
#define TOKENS 4
#define ACCOUNTS 4

#include <apps/trade_execution.c>
#include "util.h"

TEST(TradeExecutionTest, ValidInput) {
    struct In input {0, 0, 0, 0};
    struct Out output;
    compute(&input, &output);
}

int main(int argc, char **argv) {
    testing::FLAGS_gtest_death_test_style = "threadsafe";
    libsnark::default_r1cs_ppzksnark_pp::init_public_params();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}