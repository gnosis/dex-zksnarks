#include <stdint.h>

#ifndef TEST
#define ORDERS 100
#define ACCOUNTS 100
#define TOKENS 30
#endif

#include "macros.h"

struct Private {
    field254 orders[ORDERS*BITS_PER_ORDER];
    field254 pricesAndVolumes[(TOKENS*BITS_PER_DECIMAL)+(3*ORDERS*BITS_PER_DECIMAL)];
    field254 balances[ACCOUNTS*TOKENS*BITS_PER_DECIMAL];
};

struct In {
    field254 state;
    field254 surplus;
    field254 hashBatchInfo[2];
    field254 orderHash;
    field254 one;
};

struct Out { field254 state; };