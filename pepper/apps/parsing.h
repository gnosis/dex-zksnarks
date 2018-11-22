#include <stdint.h>
#include "dex_common.h"
#include "macros.h"

#define BITS_PER_ACCOUNT BITS_TO_REPRESENT(ACCOUNTS)
#define BITS_PER_TOKEN BITS_TO_REPRESENT(TOKENS)
#define BITS_PER_DECIMAL 100
#define BITS_PER_ORDER (BITS_PER_ACCOUNT + (2 * BITS_PER_TOKEN) + (2 * BITS_PER_DECIMAL))

struct Order {
    uint32_t account;
    uint32_t sellToken;
    uint32_t buyToken;
    uint32_t limitPrice;
    uint32_t sellAmount;
};

struct Balance {
    uint16_t token[TOKENS];
};

uint32_t parseAccount(bool* bits) {
    return sumBits(bits, BITS_PER_ACCOUNT);
}

uint32_t parseToken(bool* bits) {
    return sumBits(bits, BITS_PER_TOKEN);
}

uint32_t parseDecimal(bool* bits) {
    return sumBits(bits, BITS_PER_DECIMAL);
}

struct Order parseOrder(bool *bits) {
    return {
        parseAccount(bits),
        parseToken(bits + BITS_PER_ACCOUNT),
        parseToken(bits + BITS_PER_ACCOUNT + BITS_PER_TOKEN),
        parseDecimal(bits + BITS_PER_ACCOUNT + BITS_PER_TOKEN + BITS_PER_TOKEN),
        parseDecimal(bits + BITS_PER_ACCOUNT + BITS_PER_TOKEN + BITS_PER_TOKEN + BITS_PER_DECIMAL)
    };
}

void parseOrders(bool *bits, struct Order orders[ORDERS]) {
    uint32_t index = 0;
    for (index = 0; index < ORDERS; index++) {
        orders[index] = parseOrder(bits + (index * BITS_PER_ORDER));
    }
}

void parseBalances(bool *bits, struct Balance balances[ACCOUNTS]) {
    uint32_t accountIndex, tokenIndex = 0;
    for (accountIndex = 0; accountIndex < ACCOUNTS; accountIndex++) {
        for (tokenIndex = 0; tokenIndex < TOKENS; tokenIndex++) {
            uint32_t offset = BITS_PER_DECIMAL * (accountIndex * TOKENS + tokenIndex);
            balances[accountIndex].token[tokenIndex] = parseDecimal(bits + offset);
        }
    }
}

void serializeBalances(struct Balance balances[ACCOUNTS], bool* serialized) {
    uint32_t accountIndex, tokenIndex = 0;
    for (accountIndex = 0; accountIndex < ACCOUNTS; accountIndex++) {
        for (tokenIndex = 0; tokenIndex < TOKENS; tokenIndex++) {
            bool serializedToken[254] = { 0 };
            decomposeBits(balances[accountIndex].token[tokenIndex], serializedToken);
            uint32_t offset = BITS_PER_DECIMAL * (accountIndex * TOKENS + tokenIndex);
            copyBits(serializedToken, 154, serialized, offset, BITS_PER_DECIMAL);
        }
    }
}