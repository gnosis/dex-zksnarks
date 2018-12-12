#include <cassert>
#include <gtest/gtest.h>
#include "declarations.h"

struct Private {};
#define ORDERS 2
#define TOKENS 4
#define ACCOUNTS 4

#include <apps/parsing.h>
#include "util.h"

TEST(ParsingTest, ParseAccount) {
    field254 bits[BITS_PER_ACCOUNT] = { 0 };
    auto account = parseAccount(bits, 0);
    ASSERT_EQ(account, 0);

    bits[BITS_PER_ACCOUNT-1] = 1;
    account = parseAccount(bits, 0);
    ASSERT_EQ(account, 1);

    for (size_t i = 0; i < BITS_PER_ACCOUNT; i++) {
        bits[i] = 1;
    }
    account = parseAccount(bits, 0);
    ASSERT_EQ(account, ACCOUNTS - 1);
}

TEST(ParsingTest, ParseToken) {
    field254 bits[BITS_PER_TOKEN] = { 0 };
    auto token = parseToken(bits, 0);
    ASSERT_EQ(token, 0);

    bits[BITS_PER_TOKEN-1] = 1;
    token = parseToken(bits, 0);
    ASSERT_EQ(token, 1);

    for (size_t i = 0; i < BITS_PER_TOKEN; i++) {
        bits[i] = 1;
    }
    token = parseToken(bits, 0);
    ASSERT_EQ(token, ACCOUNTS - 1);
}

TEST(ParsingTest, ParseDecimal) {
    field254 bits[BITS_PER_DECIMAL] = { 0 };
    auto decimal = parseDecimal(bits, 0);
    ASSERT_EQ(decimal, 0);

    bits[BITS_PER_DECIMAL-1] = 1;
    decimal = parseDecimal(bits, 0);
    ASSERT_EQ(decimal, 1);
}

TEST(ParsingTest, ParseOrders) {
    field254 bits[BITS_PER_ORDER*ORDERS] = { 0 };
    
    /**
     * First order: {
     *   account: 1,
     *   sellToken: 1
     *   buyToken: 2
     *   buyAmount: 4
     *   sellAmount: 8
     * }
     */
    bits[BITS_PER_ACCOUNT-1] = 1;
    bits[BITS_PER_ACCOUNT + BITS_PER_TOKEN - 1] = 1;
    bits[BITS_PER_ACCOUNT + (2*BITS_PER_TOKEN) - 2] = 1;
    bits[BITS_PER_ACCOUNT + (2*BITS_PER_TOKEN) + BITS_PER_DECIMAL - 3] = 1;
    bits[BITS_PER_ACCOUNT + (2*BITS_PER_TOKEN) + (2*BITS_PER_DECIMAL) - 4] = 1;

    /**
     * Second: {
     *   account: 2,
     *   sellToken: 2
     *   buyToken: 1
     *   buyAmount: 16
     *   sellAmount: 32
     * }
     */
    bits[BITS_PER_ORDER + BITS_PER_ACCOUNT - 2] = 1;
    bits[BITS_PER_ORDER + BITS_PER_ACCOUNT + BITS_PER_TOKEN - 2] = 1;
    bits[BITS_PER_ORDER + BITS_PER_ACCOUNT + (2*BITS_PER_TOKEN) - 1] = 1;
    bits[BITS_PER_ORDER + BITS_PER_ACCOUNT + (2*BITS_PER_TOKEN) + BITS_PER_DECIMAL - 5] = 1;
    bits[BITS_PER_ORDER + BITS_PER_ACCOUNT + (2*BITS_PER_TOKEN) + (2*BITS_PER_DECIMAL) - 6] = 1;

    struct Order result[ORDERS];
    parseOrders(bits, 0, result);

    ASSERT_EQ(result[0].account, 1);
    ASSERT_EQ(result[0].sellToken, 1);
    ASSERT_EQ(result[0].buyToken, 2);
    ASSERT_EQ(result[0].buyAmount, 4);
    ASSERT_EQ(result[0].sellAmount, 8);

    ASSERT_EQ(result[1].account, 2);
    ASSERT_EQ(result[1].sellToken, 2);
    ASSERT_EQ(result[1].buyToken, 1);
    ASSERT_EQ(result[1].buyAmount, 16);
    ASSERT_EQ(result[1].sellAmount, 32);
}

TEST(ParsingTest, ParseBalances) {
    field254 bits[ACCOUNTS*TOKENS*BITS_PER_DECIMAL] = { 0 };
    
    // Each account will have balance in a single token 
    // (same ID as their account)
    bits[BITS_PER_DECIMAL - 1] = 1; //acc1
    bits[(TOKENS*BITS_PER_DECIMAL) + (2*BITS_PER_DECIMAL) - 1] = 1; //acc2
    bits[(2*TOKENS*BITS_PER_DECIMAL) + (3*BITS_PER_DECIMAL) - 1] = 1; //acc3
    bits[(3*TOKENS*BITS_PER_DECIMAL) + (4*BITS_PER_DECIMAL) - 1] = 1; //acc4

    struct Balance balances[ACCOUNTS] = { 0 };
    parseBalances(bits, 0, balances);

    ASSERT_EQ(balances[0].token[0], 1);
    ASSERT_EQ(balances[0].token[1], 0);
    ASSERT_EQ(balances[0].token[2], 0);
    ASSERT_EQ(balances[0].token[3], 0);

    ASSERT_EQ(balances[1].token[0], 0);
    ASSERT_EQ(balances[1].token[1], 1);
    ASSERT_EQ(balances[1].token[2], 0);
    ASSERT_EQ(balances[1].token[3], 0);

    ASSERT_EQ(balances[2].token[0], 0);
    ASSERT_EQ(balances[2].token[1], 0);
    ASSERT_EQ(balances[2].token[2], 1);
    ASSERT_EQ(balances[2].token[3], 0);

    ASSERT_EQ(balances[3].token[0], 0);
    ASSERT_EQ(balances[3].token[1], 0);
    ASSERT_EQ(balances[3].token[2], 0);
    ASSERT_EQ(balances[3].token[3], 1);
}

TEST(ParsingTest, SerializeBalances) {
    field254 original[ACCOUNTS*TOKENS*BITS_PER_DECIMAL] = { 0 };
    original[BITS_PER_DECIMAL - 1] = 1;
    original[(TOKENS*BITS_PER_DECIMAL) + (2*BITS_PER_DECIMAL) - 1] = 1;
    original[(2*TOKENS*BITS_PER_DECIMAL) + (3*BITS_PER_DECIMAL) - 1] = 1;
    original[(3*TOKENS*BITS_PER_DECIMAL) + (4*BITS_PER_DECIMAL) - 1] = 1;

    struct Balance balances[ACCOUNTS] = { 0 };
    parseBalances(original, 0, balances);

    // serialize back and expect to match original
    field254 serialized[ACCOUNTS*TOKENS*BITS_PER_DECIMAL] = { 0 };
    serializeBalances(balances, serialized, 0);

    ASSERT_EQ(
        std::vector<field254>(original, original+(ACCOUNTS*TOKENS*BITS_PER_DECIMAL)),
        std::vector<field254>(serialized, serialized+(ACCOUNTS*TOKENS*BITS_PER_DECIMAL))
    );
}

TEST(ParsingTest, ParsePrices) {
    field254 bits[TOKENS*BITS_PER_DECIMAL] = { 0 };
    
    // Each token will have price 2^tokenIndex
    bits[BITS_PER_DECIMAL - 1] = 1; //token1
    bits[(2*BITS_PER_DECIMAL) - 2] = 1; //token2
    bits[(3*BITS_PER_DECIMAL) - 3] = 1; //token3
    bits[(4*BITS_PER_DECIMAL) - 4] = 1; //token4

    field254 prices[TOKENS] = { 0 };
    parsePrices(bits, 0, prices);

    ASSERT_EQ(prices[0], 1);
    ASSERT_EQ(prices[1], 2);
    ASSERT_EQ(prices[2], 4);
    ASSERT_EQ(prices[3], 8);
}

TEST(ParsingTest, ParseVolumes) {
    field254 bits[2*ORDERS*BITS_PER_DECIMAL] = { 0 };
    
    // Buy and sell volume will be the same for each order (2^^orderIndex)
    bits[BITS_PER_DECIMAL - 1] = 1; //sellVolume order1
    bits[(ORDERS*BITS_PER_DECIMAL) + BITS_PER_DECIMAL - 1] = 1; //buyVolume order1
    bits[(2*BITS_PER_DECIMAL) - 2] = 1; //sellVolume order2
    bits[(ORDERS*BITS_PER_DECIMAL) + (2 * BITS_PER_DECIMAL) - 2] = 1; //sellVolume order2

    struct Volume volumes[ORDERS] = { 0 };
    parseVolumes(bits, 0, volumes);

    ASSERT_EQ(volumes[0].sellVolume, 1);
    ASSERT_EQ(volumes[0].buyVolume, 1);
    ASSERT_EQ(volumes[1].sellVolume, 2);
    ASSERT_EQ(volumes[1].buyVolume, 2);
}

int main(int argc, char **argv) {
    testing::FLAGS_gtest_death_test_style = "threadsafe";
    libsnark::default_r1cs_ppzksnark_pp::init_public_params();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}