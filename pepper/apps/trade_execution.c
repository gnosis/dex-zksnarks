#include "trade_execution.h"
#include "dex_common.h"
#include "parsing.h"
#include "hashing.h"

void compute(struct In *input, struct Out *output) {
    struct Private pInput = readPrivateInput(2);

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
    field254 balances[ACCOUNTS*TOKENS] = {0};
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
        
        // Verify volume has roughly same ratio as prices
        field254 lhs = volume.buyVolume * prices[fieldToInt(order.buyToken)];
        field254 rhs = volume.sellVolume * prices[fieldToInt(order.sellToken)];
        field254 delta = lhs - rhs;
        // Make sure |delta| < epsilon
        assert_zero(isNegative((input->epsilon*input->epsilon) - delta) || isNegative((input->epsilon*input->epsilon) + delta));

        // Limit price compliance
        assert_zero(isNegative(fieldToInt(volume.sellVolume * order.buyAmount) - fieldToInt(volume.buyVolume * order.sellAmount)));
        // Limit amount compliance
        assert_zero(isNegative(order.sellAmount - volume.sellVolume));

        // Verify surplus
        assert_zero(isNegative((((volume.buyVolume * order.sellAmount) - (volume.sellVolume * order.buyAmount)) * prices[fieldToInt(order.buyToken)]) - (volume.surplus * order.sellAmount)));
        totalSurplus += volume.surplus;
        
        balances[(fieldToInt(order.account) * TOKENS) + fieldToInt(order.sellToken)] -= volume.sellVolume;
        balances[(fieldToInt(order.account) * TOKENS) + fieldToInt(order.buyToken)] += volume.buyVolume;

        buyVolumes[fieldToInt(order.buyToken)] += volume.buyVolume;
        sellVolumes[fieldToInt(order.sellToken)] += volume.sellVolume;
    }
    // check that buyVolume ≈≈ sellVolume for each token, sellVolume cannot be smaller
    for (index=0; index < TOKENS; index++) {
        assert_zero(isNegative(sellVolumes[index] - buyVolumes[index])); 
        assert_zero(isNegative(input->epsilon + buyVolumes[index] - sellVolumes[index]));
    }
    assert_zero(totalSurplus - input->surplus);

    for (index = 0; index < ACCOUNTS; index++) {
        uint32_t tokenIndex;
        for (tokenIndex = 0; tokenIndex < TOKENS; tokenIndex++) {
            assert_zero(isNegative(balances[(index * TOKENS) + tokenIndex]));
        }
    }

    // decompose and rehash updated balances to output
    field254 updatedBalances[ACCOUNTS*TOKENS*100];
    serializeBalances(balances, updatedBalances, 0);
    output->state = hashPedersen(updatedBalances, 0, ACCOUNTS*TOKENS*100, 100);
}
