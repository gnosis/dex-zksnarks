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
        
        field254 surplus = volume.surplus;
        //field254 surplus = computeSurplus(order, prices);
        if (fieldToInt(surplus) > 0) {
            // Make sure volume has same ratio as prices
            //validateVolume(volume, order, prices);
//            assert_zero((volume.sellVolume * prices[fieldToInt(order.buyToken)]) - (volume.buyVolume * prices[fieldToInt(order.buyToken)]));
            totalSurplus += surplus;
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
    assert_zero(totalSurplus - input->surplus);

    for (index = 0; index < ACCOUNTS; index++) {
        uint32_t tokenIndex;
        for (tokenIndex = 0; tokenIndex < TOKENS; tokenIndex++) {
            if (fieldToInt(balances[index].token[tokenIndex]) < 0) {
                assert_zero(input->one); // HACK we need a wait to fail here.
            }
        }
    }

    // decompose and rehash updated balances to output
    field254 updatedBalances[ACCOUNTS*TOKENS*100];
    serializeBalances(balances, updatedBalances, 0);
    output->state = hashPedersen(updatedBalances, 0, ACCOUNTS*TOKENS*100, 100);
}
