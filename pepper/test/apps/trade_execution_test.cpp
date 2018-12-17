#include <cassert>
#include <gtest/gtest.h>
#include "declarations.h"

#define ORDERS 2
#define TOKENS 4
#define ACCOUNTS 4

#include <apps/trade_execution.c>
#include "util.h"

struct Private serializePrivateInput(struct Balance balances[ACCOUNTS], struct Order orders[ORDERS], struct Volume volumes[ORDERS], field254 prices[TOKENS]) {
    Private result = {};
    serializeOrders(orders, result.orders, 0);
    serializePricesAndVolumes(prices, volumes, result.pricesAndVolumes, 0);
    serializeBalances(balances, result.balances, 0);
    return result;
}

#pragma mark - Valid instances

TEST(TradeExecutionTest, ValidEmptyBatch) {
    struct Balance balances[ACCOUNTS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {0};

    privateInput = serializePrivateInput(balances, orders, volumes, prices);
    field254 state = hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL);
    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        state,
        0, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
        1 //one
    };
    struct Out output;

    compute(&input, &output);
    ASSERT_EQ(output.state, state);
}

TEST(TradeExecutionTest, ValidSingleOrderTrade) {
    struct Balance balances[ACCOUNTS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {1, 1, 1, 1};

    // First order
    orders[0].account = 0;
    orders[0].sellToken = 1;
    orders[0].buyToken = 2;
    orders[0].sellAmount = 100;
    orders[0].buyAmount = 99;

    // second order
    orders[1].account = 1;
    orders[1].sellToken = 2;
    orders[1].buyToken = 1;
    orders[1].sellAmount = 100;
    orders[1].buyAmount = 99;

    // both are fully matched with 1 surplus
    volumes[0].sellVolume = 100;
    volumes[0].buyVolume = 100;
    volumes[0].surplus = 1;
    volumes[1].sellVolume = 100;
    volumes[1].buyVolume = 100;
    volumes[1].surplus = 1;

    //Make sure there is balance
    balances[0].token[1] = orders[0].sellAmount;
    balances[1].token[2] = orders[1].sellAmount;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        2, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
        1 //one
    };
    struct Out output;
    compute(&input, &output);

    // Execute trade and check that output matches
    balances[0].token[1] = 0;
    balances[0].token[2] = 100;
    balances[1].token[1] = 100;
    balances[1].token[2] = 0;
    field254* updatedBalances = serializePrivateInput(balances, orders, volumes, prices).balances;
    field254 updatedHash = hashPedersen(updatedBalances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL);
    ASSERT_EQ(output.state, updatedHash);
}

#pragma mark - Invalid instances

TEST(TradeExecutionTest, InvalidBalanceHash) {
    struct Balance balances[ACCOUNTS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {0};

    privateInput = serializePrivateInput(balances, orders, volumes, prices);
    field254 state = hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL);
    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        state + 1,
        0, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
        1 //one
    };
    struct Out output;

    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, InvalidOrderHash) {
    struct Balance balances[ACCOUNTS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {0};

    privateInput = serializePrivateInput(balances, orders, volumes, prices);
    field254 state = hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL);
    field254 orderHash = hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER);
    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        state,
        0, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        orderHash + 1,
        1 //one
    };
    struct Out output;

    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, InvalidBatchInfoHash) {
    struct Balance balances[ACCOUNTS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {0};

    privateInput = serializePrivateInput(balances, orders, volumes, prices);
    field254 state = hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL);
    field254 orderHash = hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER);
    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        state,
        0, //surplus
        {hashBatchInfo.left - 1, hashBatchInfo.right}, //hashBatchInfo
        orderHash,
        1 //one
    };
    struct Out output;

    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, InvalidTotalSurplus) {
    struct Balance balances[ACCOUNTS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {0};

    privateInput = serializePrivateInput(balances, orders, volumes, prices);
    field254 state = hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL);
    field254 orderHash = hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER);
    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        state,
        100, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        orderHash,
        1 //one
    };
    struct Out output;

    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, InvalidOneInput) {
    struct Balance balances[ACCOUNTS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {0};

    privateInput = serializePrivateInput(balances, orders, volumes, prices);
    field254 state = hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL);
    field254 orderHash = hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER);
    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        state,
        0, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        orderHash,
        0 //one
    };
    struct Out output;

    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, InvalidVolumeSurplus) {
    struct Balance balances[ACCOUNTS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {1, 1, 1, 1};

    // First order
    orders[0].account = 0;
    orders[0].sellToken = 1;
    orders[0].buyToken = 2;
    orders[0].sellAmount = 100;
    orders[0].buyAmount = 99;

    // second order
    orders[1].account = 1;
    orders[1].sellToken = 2;
    orders[1].buyToken = 1;
    orders[1].sellAmount = 100;
    orders[1].buyAmount = 99;

    // both are fully matched with incorrect surplus
    volumes[0].sellVolume = 100;
    volumes[0].buyVolume = 100;
    volumes[0].surplus = 100;
    volumes[1].sellVolume = 100;
    volumes[1].buyVolume = 100;
    volumes[1].surplus = 100;

    //Make sure there is balance
    balances[0].token[1] = orders[0].sellAmount;
    balances[1].token[2] = orders[1].sellAmount;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        2, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
        1 //one
    };
    struct Out output;
    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, NotEnoughBalance) {
    struct Balance balances[ACCOUNTS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {1, 1, 1, 1};

    // First order
    orders[0].account = 0;
    orders[0].sellToken = 1;
    orders[0].buyToken = 2;
    orders[0].sellAmount = 100;
    orders[0].buyAmount = 99;

    // second order
    orders[1].account = 1;
    orders[1].sellToken = 2;
    orders[1].buyToken = 1;
    orders[1].sellAmount = 100;
    orders[1].buyAmount = 99;

    // both are fully matched with 1 surplus
    volumes[0].sellVolume = 100;
    volumes[0].buyVolume = 100;
    volumes[0].surplus = 1;
    volumes[1].sellVolume = 100;
    volumes[1].buyVolume = 100;
    volumes[1].surplus = 1;

    //Not enough balance
    balances[0].token[1] = orders[0].sellAmount - 1;
    balances[1].token[2] = orders[1].sellAmount;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        2, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
        1 //one
    };
    struct Out output;
    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, LimitPriceIgnored) {
    struct Balance balances[ACCOUNTS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {1, 1, 1, 1};

    // First order
    orders[0].account = 0;
    orders[0].sellToken = 1;
    orders[0].buyToken = 2;
    orders[0].sellAmount = 100;
    orders[0].buyAmount = 101;

    // second order
    orders[1].account = 1;
    orders[1].sellToken = 2;
    orders[1].buyToken = 1;
    orders[1].sellAmount = 100;
    orders[1].buyAmount = 99;

    // both are fully matched with 1 surplus
    volumes[0].sellVolume = 100;
    volumes[0].buyVolume = 100;
    volumes[0].surplus = 1;
    volumes[1].sellVolume = 100;
    volumes[1].buyVolume = 100;
    volumes[1].surplus = 1;

    //Make sure there is balance
    balances[0].token[1] = orders[0].sellAmount;
    balances[1].token[2] = orders[1].sellAmount;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        2, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
        1 //one
    };
    struct Out output;
    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, TotalBoughtAndSoldDontMatch) {
    struct Balance balances[ACCOUNTS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {100, 100, 100, 100};

    // First order
    orders[0].account = 0;
    orders[0].sellToken = 1;
    orders[0].buyToken = 2;
    orders[0].sellAmount = 100;
    orders[0].buyAmount = 99;

    // second order
    orders[1].account = 1;
    orders[1].sellToken = 2;
    orders[1].buyToken = 1;
    orders[1].sellAmount = 100;
    orders[1].buyAmount = 99;

    // Sell and buy volumes don't match
    volumes[0].sellVolume = 50;
    volumes[0].buyVolume = 50;
    volumes[0].surplus = 50;
    volumes[1].sellVolume = 100;
    volumes[1].buyVolume = 100;
    volumes[1].surplus = 100;

    //Make sure there is balance
    balances[0].token[1] = orders[0].sellAmount;
    balances[1].token[2] = orders[1].sellAmount;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        150, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
        1 //one
    };
    struct Out output;
    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, MoreVolumeThanAuthorized) {
    struct Balance balances[ACCOUNTS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {1, 1, 1, 1};

    // First order
    orders[0].account = 0;
    orders[0].sellToken = 1;
    orders[0].buyToken = 2;
    orders[0].sellAmount = 50;
    orders[0].buyAmount = 49;

    // second order
    orders[1].account = 1;
    orders[1].sellToken = 2;
    orders[1].buyToken = 1;
    orders[1].sellAmount = 100;
    orders[1].buyAmount = 99;

    // More volume than authorized
    volumes[0].sellVolume = 100;
    volumes[0].buyVolume = 100;
    volumes[0].surplus = 2;
    volumes[1].sellVolume = 100;
    volumes[1].buyVolume = 100;
    volumes[1].surplus = 1;

    //Make sure there is balance
    balances[0].token[1] = volumes[0].sellVolume;
    balances[1].token[2] = volumes[1].sellVolume;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        3, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
        1 //one
    };
    struct Out output;
    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, PriceDoesntMatchVolume) {
    struct Balance balances[ACCOUNTS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {10, 5, 10, 10};

    // First order
    orders[0].account = 0;
    orders[0].sellToken = 1;
    orders[0].buyToken = 2;
    orders[0].sellAmount = 100;
    orders[0].buyAmount = 99;

    // second order
    orders[1].account = 1;
    orders[1].sellToken = 2;
    orders[1].buyToken = 1;
    orders[1].sellAmount = 100;
    orders[1].buyAmount = 99;

    // More volume than authorized
    volumes[0].sellVolume = 100;
    volumes[0].buyVolume = 100;
    volumes[0].surplus = 10;
    volumes[1].sellVolume = 100;
    volumes[1].buyVolume = 100;
    volumes[1].surplus = 5;

    //Make sure there is balance
    balances[0].token[1] = volumes[0].sellVolume;
    balances[1].token[2] = volumes[1].sellVolume;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        15, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
        1 //one
    };
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