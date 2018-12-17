#include <stdint.h>
#include "dex_common.h"
#include "macros.h"

struct Order {
    field254 account;
    field254 sellToken;
    field254 buyToken;
    field254 sellAmount;
    field254 buyAmount;
};

struct Balance {
    field254 token[TOKENS];
};

struct Volume {
    field254 sellVolume;
    field254 buyVolume;
    field254 surplus;
};

field254 parseAccount(field254* bits, uint32_t offset) {
    return sumBits(bits, offset, BITS_PER_ACCOUNT);
}

field254 parseToken(field254* bits, uint32_t offset) {
    return sumBits(bits, offset, BITS_PER_TOKEN);
}

field254 parseDecimal(field254* bits, uint32_t offset) {
    return sumBits(bits, offset, BITS_PER_DECIMAL);
}

struct Order parseOrder(field254 *bits, uint32_t offset) {
    struct Order order = {
        parseAccount(bits, offset),
        parseToken(bits, offset + BITS_PER_ACCOUNT),
        parseToken(bits, offset + BITS_PER_ACCOUNT + BITS_PER_TOKEN),
        parseDecimal(bits, offset + BITS_PER_ACCOUNT + BITS_PER_TOKEN + BITS_PER_TOKEN),
        parseDecimal(bits, offset + BITS_PER_ACCOUNT + BITS_PER_TOKEN + BITS_PER_TOKEN + BITS_PER_DECIMAL)
    };
    return order;
}

void parseOrders(field254 *bits, uint32_t offset, struct Order orders[ORDERS]) {
    uint32_t index = 0;
    for (index = 0; index < ORDERS; index++) {
        orders[index] = parseOrder(bits, offset + (index * BITS_PER_ORDER));
    }
}

void serializeOrders(struct Order orders[ORDERS], field254 *serialized, uint32_t offset) {
    uint32_t index = 0;
    for (index = 0; index < ORDERS; index++) {
        decomposeBits(orders[index].account, serialized, offset, BITS_PER_ACCOUNT);
        offset += BITS_PER_ACCOUNT;
        decomposeBits(orders[index].sellToken, serialized, offset, BITS_PER_TOKEN);
        offset += BITS_PER_TOKEN;
        decomposeBits(orders[index].buyToken, serialized, offset, BITS_PER_TOKEN);
        offset += BITS_PER_TOKEN;
        decomposeBits(orders[index].sellAmount, serialized, offset, BITS_PER_DECIMAL);
        offset += BITS_PER_DECIMAL;
        decomposeBits(orders[index].buyAmount, serialized, offset, BITS_PER_DECIMAL);
        offset += BITS_PER_DECIMAL;
    }
}

void parseBalances(field254 *bits, uint32_t offset, struct Balance balances[ACCOUNTS]) {
    uint32_t accountIndex, tokenIndex = 0;
    for (accountIndex = 0; accountIndex < ACCOUNTS; accountIndex++) {
        for (tokenIndex = 0; tokenIndex < TOKENS; tokenIndex++) {
            uint32_t balanceOffset = offset + (BITS_PER_DECIMAL * (accountIndex * TOKENS + tokenIndex));
            balances[accountIndex].token[tokenIndex] = parseDecimal(bits, balanceOffset);
        }
    }
}

void serializeBalances(struct Balance balances[ACCOUNTS], field254* serialized, uint32_t offset) {
    uint32_t accountIndex, tokenIndex = 0;
    for (accountIndex = 0; accountIndex < ACCOUNTS; accountIndex++) {
        for (tokenIndex = 0; tokenIndex < TOKENS; tokenIndex++) {
            uint32_t balanceOffset = offset + (BITS_PER_DECIMAL * (accountIndex * TOKENS + tokenIndex));
            decomposeBits(balances[accountIndex].token[tokenIndex], serialized, balanceOffset, BITS_PER_DECIMAL);
        }
    }
}

void parsePrices(field254 *bits, uint32_t offset, field254 prices[TOKENS]) {
    uint32_t tokenIndex = 0;
    for (tokenIndex = 0; tokenIndex < TOKENS; tokenIndex++) {
        prices[tokenIndex] = parseDecimal(bits, offset + (tokenIndex * BITS_PER_DECIMAL));
    }
}

void parseVolumes(field254 *bits, uint32_t offset, struct Volume volumes[ORDERS]) {
    uint32_t index = 0;
    for (index = 0; index < ORDERS; index++) {
        // For hash verification efficiency all buyVolumes are adjacent:
        // [sellVolumeOrder1, .., sellVolumeOrderN, buyVolumeOrder1, .., buyVolumeOrderN, surpleVolumeOrder1, .., suprlusVolumeOrderN ]
        field254 sellVolume = parseDecimal(bits, offset + (index * BITS_PER_DECIMAL));
        field254 buyVolume = parseDecimal(bits, offset + ((ORDERS + index) * BITS_PER_DECIMAL));
        field254 surplus = parseDecimal(bits, offset + ((2 * ORDERS + index) * BITS_PER_DECIMAL));
        volumes[index].sellVolume = sellVolume;
        volumes[index].buyVolume = buyVolume;
        volumes[index].surplus = surplus;
    }
}

void serializePricesAndVolumes(field254 prices[TOKENS], struct Volume volumes[ORDERS], field254 *bits, uint32_t offset) {
    uint32_t index = 0;
    for (index = 0; index < TOKENS; index++) {
        decomposeBits(prices[index], bits, offset, BITS_PER_DECIMAL);
        offset += BITS_PER_DECIMAL;
    }
    for (index = 0; index < ORDERS; index++) {
        decomposeBits(volumes[index].sellVolume, bits, offset, BITS_PER_DECIMAL);
        decomposeBits(volumes[index].buyVolume, bits, offset + (ORDERS * BITS_PER_DECIMAL), BITS_PER_DECIMAL);
        decomposeBits(volumes[index].surplus, bits, offset + (2 * ORDERS * BITS_PER_DECIMAL), BITS_PER_DECIMAL);
        offset += BITS_PER_DECIMAL;
    }
}