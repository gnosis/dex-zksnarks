#include "trade_execution.h"
#include "dex_common.h"
#include "parsing.h"
#include "hashing.h"

field254 computeSurplus(struct Order order, field254 prices[TOKENS]) {
    // buyAmount/sellAmount >= price(buyToken->sellToken)
    // buyAmount/sellAmount - surplus == price(buyToken->sellToken)
    // buyAmount/sellAmount - surplus == price(buyToken->ref)/price(sellToken->ref)
    // buyAmount * price(sellToken->ref) - surplus * price(sellToken->ref) * sellAmount = price(buyToken->ref) * sellVolume
    // buyAmount * price(sellToken->ref) - price(buyToken->ref) * sellAmount = surplus * price(sellToken->ref) * sellAmount
    return (order.buyAmount * prices[fieldToInt(order.sellToken)]) - (order.sellAmount * prices[fieldToInt(order.buyToken)]);
}

void validateVolume(struct Volume volume, struct Order order, field254 prices[TOKENS]) {
    //     buyVolume / sellVolume == price(buyToken->sellToken)
    // <=> buyVolume / sellVolume == price(buyToken->ref) / price(sellToken->ref)
    // <=> buyVolume * price(sellToken->ref) == price(buyToken->ref) * sellVolume
    assert_zero((volume.sellVolume * prices[fieldToInt(order.buyToken)]) - (volume.buyVolume * prices[fieldToInt(order.buyToken)]));
}

void compute(struct In *input, struct Out *output) {
    struct Private pInput = readPrivateInput();

    // Assert that private input matches public
    // Step 1: Balances (PedersenHash)
    field254 balanceHashPedersen = hashPedersen(pInput.balances, ACCOUNTS*TOKENS*100, 100);
    assert_zero(input->state - balanceHashPedersen);
    
    // Step 2: Orders (PedersenHash)
    field254 orderHashPedersen = hashPedersen(pInput.orders, ORDERS*253, 253);
    assert_zero(input->orderHash - orderHashPedersen);
    
    // Step 3: Prices/Volumes (SHA Hash)
    struct ShaResult hashBatchInfo = hashSHA(pInput.pricesAndVolumes, (TOKENS*100)+(ORDERS*100), 256);
    assert_zero(input->hashBatchInfo - hashBatchInfo.left);
    assert_zero(input->hashBatchInfo - hashBatchInfo.right);

    // Parse all private input
    struct Order orders[ORDERS] = {0};
    parseOrders(pInput.orders, orders);
    struct Balance balances[ACCOUNTS] = {0};
    parseBalances(pInput.balances, balances);
    field254 prices[TOKENS] = {0};
    parsePrices(pInput.pricesAndVolumes, prices);
    struct Volume volumes[ORDERS] = {0};
    parseVolumes(pInput.pricesAndVolumes + (TOKENS*100), volumes);

    // Accumulator variables
    field254 buyVolumes[TOKENS]= {0};
    field254 sellVolumes[TOKENS]= {0};
    field254 totalSurplus = 0;

    // validate each order and update balances
    uint32_t index;
    for (index=0; index < ORDERS; index++) {
        struct Order order = orders[index];
        struct Volume volume = volumes[index];
        
        field254 surplus = (order.buyAmount * prices[fieldToInt(order.sellToken)]) - (order.sellAmount * prices[fieldToInt(order.buyToken)]);
        //field254 surplus = computeSurplus(order, prices);
        if (fieldToInt(surplus) > 0) {
            // Make sure volume has same ratio as prices
            validateVolume(volume, order, prices);
            totalSurplus += surplus * volume.sellVolume;
        } else {
            // Order should not be touched
            assert_zero(volume.sellVolume);
            assert_zero(volume.buyVolume);
        }
        
        balances[fieldToInt(order.account)].token[fieldToInt(order.sellToken)] -= volume.sellVolume;
        balances[fieldToInt(order.account)].token[fieldToInt(order.buyToken)] += volume.buyVolume;

        buyVolumes[fieldToInt(order.buyToken)] += volume.buyVolume;
        sellVolumes[fieldToInt(order.sellToken)] += volume.sellVolume;
    }

    // check that buyVolume == sellVolume for each token
    for (index=0; index < TOKENS; index++) {
        assert_zero(buyVolumes[index] - sellVolumes[index]);
    }
    assert_zero(totalSurplus - input->welfare);
    
    // decompose and rehash updated balances to output
    field254 updatedBalances[ACCOUNTS*TOKENS*100];
    serializeBalances(balances, updatedBalances);
    output->state = hashPedersen(updatedBalances, ACCOUNTS*TOKENS*100, 100);
}
