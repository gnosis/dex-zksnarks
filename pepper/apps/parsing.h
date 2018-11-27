#include <stdint.h>
#include "dex_common.h"
#include "macros.h"

#define BITS_PER_ACCOUNT BITS_TO_REPRESENT(ACCOUNTS)
#define BITS_PER_TOKEN BITS_TO_REPRESENT(TOKENS)
#define BITS_PER_DECIMAL 100
#define BITS_PER_ORDER (BITS_PER_ACCOUNT + (2 * BITS_PER_TOKEN) + (2 * BITS_PER_DECIMAL))

struct Order {
    field254 account;
    field254 sellToken;
    field254 buyToken;
    field254 limitPrice;
    field254 sellAmount;
};

struct Balance {
    field254 token[TOKENS];
};

struct Volume {
    field254 sellVolume;
    field254 buyVolume;
};

field254 parseAccount(field254* bits) {
    return sumBits(bits, BITS_PER_ACCOUNT);
}

field254 parseToken(field254* bits) {
    return sumBits(bits, BITS_PER_TOKEN);
}

field254 parseDecimal(field254* bits) {
    return sumBits(bits, BITS_PER_DECIMAL);
}

struct Order parseOrder(field254 *bits) {
    return {
        parseAccount(bits),
        parseToken(bits + BITS_PER_ACCOUNT),
        parseToken(bits + BITS_PER_ACCOUNT + BITS_PER_TOKEN),
        parseDecimal(bits + BITS_PER_ACCOUNT + BITS_PER_TOKEN + BITS_PER_TOKEN),
        parseDecimal(bits + BITS_PER_ACCOUNT + BITS_PER_TOKEN + BITS_PER_TOKEN + BITS_PER_DECIMAL)
    };
}

void parseOrders(field254 *bits, struct Order orders[ORDERS]) {
    uint32_t index = 0;
    for (index = 0; index < ORDERS; index++) {
        orders[index] = parseOrder(bits + (index * BITS_PER_ORDER));
    }
}

void parseBalances(field254 *bits, struct Balance balances[ACCOUNTS]) {
    uint32_t accountIndex, tokenIndex = 0;
    for (accountIndex = 0; accountIndex < ACCOUNTS; accountIndex++) {
        for (tokenIndex = 0; tokenIndex < TOKENS; tokenIndex++) {
            uint32_t offset = BITS_PER_DECIMAL * (accountIndex * TOKENS + tokenIndex);
            balances[accountIndex].token[tokenIndex] = parseDecimal(bits + offset);
        }
    }
}

void serializeBalances(struct Balance balances[ACCOUNTS], field254* serialized) {
    uint32_t accountIndex, tokenIndex = 0;
    for (accountIndex = 0; accountIndex < ACCOUNTS; accountIndex++) {
        for (tokenIndex = 0; tokenIndex < TOKENS; tokenIndex++) {
            field254 serializedToken[254] = { 0 };
            decomposeBits(balances[accountIndex].token[tokenIndex], serializedToken);
            uint32_t offset = BITS_PER_DECIMAL * (accountIndex * TOKENS + tokenIndex);
            copyBits(serializedToken, 154, serialized, offset, BITS_PER_DECIMAL);
        }
    }
}

void parsePrices(field254 *bits, field254 prices[TOKENS]) {
    uint32_t tokenIndex = 0;
    for (tokenIndex = 0; tokenIndex < TOKENS; tokenIndex++) {
        prices[tokenIndex ] = parseDecimal(bits + (tokenIndex * BITS_PER_DECIMAL));
    }
}

void parseVolumes(field254 *bits, struct Volume volumes[ORDERS]) {
    uint32_t index = 0;
    for (index = 0; index < ORDERS; index++) {
        // For hash verification efficiency all buyVolumes are adjacent:
        // [sellVolumeOrder1, .., sellVolumeOrderN, buyVolumeOrder1, .., buyVolumeOrderN ]
        field254 sellVolume = parseDecimal(bits + (index * BITS_PER_DECIMAL));
        field254 buyVolume = parseDecimal(bits + ((ORDERS + index) * BITS_PER_DECIMAL));
        volumes[index] = { sellVolume, buyVolume };
    }
}