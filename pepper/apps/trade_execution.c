#include "trade_execution.h"
#include "dex_common.h"
#include "parsing.h"
#include "hashing.h"

static const field254 EPSILON = 100;

void compute(struct In *input, struct Out *output) {
    struct Private pInput = readPrivateInput();
    // We need a way of faling an assert (e.g. assert_zero(1)).
    // Hardcoding the one in code doesn't compile, so we have to pass and verify it.
    assert_zero(input->one - 1);

    // Assert that private input matches public
    // Step 1: Balances (PedersenHash)
    field254 balanceHashPedersen = hashPedersen(pInput.balances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL);
    assert_zero(input->state - balanceHashPedersen);
    // Step 2: Orders (PedersenHash)
    field254 orderHashPedersen = hashPedersen(pInput.orders, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER);
    assert_zero(input->orderHash - orderHashPedersen);
    
    // Step 3: Prices/Volumes (SHA Hash)
    struct ShaResult hashBatchInfo = hashSHA(pInput.pricesAndVolumes, 0, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    assert_zero(input->hashBatchInfo[0] - hashBatchInfo.left);
    assert_zero(input->hashBatchInfo[1] - hashBatchInfo.right);

    // Parse all private input
    struct Order orders[ORDERS] = {0};
    parseOrders(pInput.orders, 0, orders);
    struct Balance balances[ACCOUNTS] = {0};
    parseBalances(pInput.balances, 0, balances);
    field254 prices[TOKENS] = {0};
    parsePrices(pInput.pricesAndVolumes, 0, prices);
    struct Volume volumes[ORDERS] = {0};
    parseVolumes(pInput.pricesAndVolumes, (TOKENS*100), volumes);

    // Accumulator variables
    field254 buyVolumes[TOKENS]= {0};
    field254 sellVolumes[TOKENS]= {0};
    field254 totalSurplus = 0;

    // validate each order and update balances
    uint32_t index;
    for (index=0; index < ORDERS; index++) {
        struct Order order = orders[index];
        struct Volume volume = volumes[index];
        
        if (fieldToInt(volume.sellVolume) > 0) {
            // Verify volume has same ratio as prices
            assert_zero((volume.buyVolume * prices[fieldToInt(order.buyToken)]) - (volume.sellVolume * prices[fieldToInt(order.sellToken)]));

            // Limit price compliance
            if (fieldToInt(volume.buyVolume * order.sellAmount) < fieldToInt(volume.sellVolume * order.buyAmount) || isNegative(order.sellAmount - volume.sellVolume)) {
                assert_zero(input->one);
            }

            // Verify surplus
            assert_zero((((volume.buyVolume * order.sellAmount) - (volume.sellVolume * order.buyAmount)) * prices[fieldToInt(order.buyToken)]) - (volume.surplus * order.sellAmount));
            totalSurplus += volume.surplus;
        }
        
        balances[fieldToInt(order.account)].token[fieldToInt(order.sellToken)] -= volume.sellVolume;
        balances[fieldToInt(order.account)].token[fieldToInt(order.buyToken)] += volume.buyVolume;

        buyVolumes[fieldToInt(order.buyToken)] += volume.buyVolume;
        sellVolumes[fieldToInt(order.sellToken)] += volume.sellVolume;
    }

    // check that buyVolume ≈≈ sellVolume for each token, sellVolume cannot be smaller
    for (index=0; index < TOKENS; index++) {
        if (isNegative(sellVolumes[index] - buyVolumes[index]) 
            || isNegative(EPSILON + buyVolumes[index] - sellVolumes[index])) {
            assert_zero(input->one);
        }
    }
    assert_zero(totalSurplus - input->surplus);

    for (index = 0; index < ACCOUNTS; index++) {
        uint32_t tokenIndex;
        for (tokenIndex = 0; tokenIndex < TOKENS; tokenIndex++) {
            assert_zero(isNegative(balances[index].token[tokenIndex]));
        }
    }

    // decompose and rehash updated balances to output
    field254 updatedBalances[ACCOUNTS*TOKENS*100];
    serializeBalances(balances, updatedBalances, 0);
    output->state = hashPedersen(updatedBalances, 0, ACCOUNTS*TOKENS*100, 100);
}
