#include <cassert>
#include <gtest/gtest.h>
#include "declarations.h"

#define ORDERS 2
#define TOKENS 4
#define ACCOUNTS 4

#include <apps/trade_execution.c>
#include "util.h"

struct Private serializePrivateInput(field254 balances[TOKENS * ACCOUNTS], struct Order orders[ORDERS], struct Volume volumes[ORDERS], field254 prices[TOKENS]) {
    Private result = {};
    serializeOrders(orders, result.orders, 0);
    serializePricesAndVolumes(prices, volumes, result.pricesAndVolumes, 0);
    serializeBalances(balances, result.balances, 0);
    return result;
}

#pragma mark - Valid instances

TEST(TradeExecutionTest, ValidEmptyBatch) {
    field254 balances[TOKENS * ACCOUNTS] = {0};
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
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER)
    };
    struct Out output;

    compute(&input, &output);
    ASSERT_EQ(output.state, state);
}

TEST(TradeExecutionTest, ValidSingleOrderTrade) {
    field254 balances[TOKENS * ACCOUNTS] = {0};
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
    balances[1] = orders[0].sellAmount;
    balances[TOKENS + 2] = orders[1].sellAmount;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        2, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
    };
    struct Out output;
    compute(&input, &output);

    // Execute trade and check that output matches
    balances[1] = 0;
    balances[2] = 100;
    balances[TOKENS + 1] = 100;
    balances[TOKENS + 2] = 0;
    field254* updatedBalances = serializePrivateInput(balances, orders, volumes, prices).balances;
    field254 updatedHash = hashPedersen(updatedBalances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL);
    ASSERT_EQ(output.state, updatedHash);
}

TEST(TradeExecutionTest, ValidSingleOrderRoundedTrade) {
    field254 balances[TOKENS * ACCOUNTS] = {0};
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

    // Volumes only roughly match prices,
    // we are selling one more unit than we buy
    // and providing less suprlus
    volumes[0].sellVolume = 100;
    volumes[0].buyVolume = 99;
    volumes[0].surplus = 0;
    volumes[1].sellVolume = 100;
    volumes[1].buyVolume = 100;
    volumes[1].surplus = 0;

    //Make sure there is balance
    balances[1] = volumes[0].sellVolume;
    balances[TOKENS + 2] = volumes[1].sellVolume;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        0, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER)
    };
    struct Out output;
    compute(&input, &output);

    // Execute trade and check that output matches
    balances[1] = 0;
    balances[2] = 99;
    balances[TOKENS + 1] = 100;
    balances[TOKENS + 2] = 0;
    field254* updatedBalances = serializePrivateInput(balances, orders, volumes, prices).balances;
    field254 updatedHash = hashPedersen(updatedBalances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL);
    ASSERT_EQ(output.state, updatedHash);
}

#pragma mark - Invalid instances

TEST(TradeExecutionTest, InvalidBalanceHash) {
    field254 balances[TOKENS * ACCOUNTS] = {0};
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
    };
    struct Out output;

    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, InvalidOrderHash) {
    field254 balances[TOKENS * ACCOUNTS] = {0};
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
    };
    struct Out output;

    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, InvalidBatchInfoHash) {
    field254 balances[TOKENS * ACCOUNTS] = {0};
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
    };
    struct Out output;

    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, InvalidTotalSurplus) {
    field254 balances[TOKENS * ACCOUNTS] = {0};
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
    };
    struct Out output;

    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, InvalidVolumeSurplus) {
    field254 balances[TOKENS * ACCOUNTS] = {0};
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
    balances[1] = orders[0].sellAmount;
    balances[TOKENS + 2] = orders[1].sellAmount;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        2, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
    };
    struct Out output;
    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, NotEnoughBalance) {
    field254 balances[TOKENS * ACCOUNTS] = {0};
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
    balances[1] = orders[0].sellAmount - 1;
    balances[TOKENS + 2] = orders[1].sellAmount;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        2, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
    };
    struct Out output;
    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, LimitPriceIgnored) {
    field254 balances[TOKENS * ACCOUNTS] = {0};
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
    balances[1] = orders[0].sellAmount;
    balances[TOKENS + 2] = orders[1].sellAmount;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        2, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
    };
    struct Out output;
    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, TotalBoughtAndSoldDontMatch) {
    field254 balances[TOKENS * ACCOUNTS] = {0};
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
    balances[1] = orders[0].sellAmount;
    balances[TOKENS + 2] = orders[1].sellAmount;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        150, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
    };
    struct Out output;
    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, MoreVolumeThanAuthorized) {
    field254 balances[TOKENS * ACCOUNTS] = {0};
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
    balances[1] = volumes[0].sellVolume;
    balances[TOKENS + 2] = volumes[1].sellVolume;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        3, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
    };
    struct Out output;
    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, PriceDoesntMatchVolume) {
    field254 balances[TOKENS * ACCOUNTS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {1000, 1, 1000, 1000};

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
    volumes[0].surplus = 1000;
    volumes[1].sellVolume = 100;
    volumes[1].buyVolume = 100;
    volumes[1].surplus = 1;

    //Make sure there is balance
    balances[1] = volumes[0].sellVolume;
    balances[TOKENS + 2] = volumes[1].sellVolume;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        1001, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER)
    };
    struct Out output;
    DISABLE_STACKTRACE = true;
    ASSERT_DEATH(compute(&input, &output), "");
    DISABLE_STACKTRACE = false;
}

TEST(TradeExecutionTest, BuyVolumeSlightlyGreateSellVolume) {
    field254 balances[TOKENS * ACCOUNTS] = {0};
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

    // Buy volume is slightly higher than sellVolume
    volumes[0].sellVolume = 100;
    volumes[0].buyVolume = 101;
    volumes[0].surplus = 1;
    volumes[1].sellVolume = 100;
    volumes[1].buyVolume = 100;
    volumes[1].surplus = 1;

    //Make sure there is balance
    balances[1] = orders[0].sellAmount;
    balances[TOKENS + 2] = orders[1].sellAmount;

    //Provide private input
    privateInput = serializePrivateInput(balances, orders, volumes, prices);

    ShaResult hashBatchInfo = hashSHA(privateInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    struct In input {
        hashPedersen(privateInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL), // state
        2, //surplus
        {hashBatchInfo.left, hashBatchInfo.right}, //hashBatchInfo
        hashPedersen(privateInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER),
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