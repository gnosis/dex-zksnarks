#include <stdint.h>

#ifndef TEST
#define ORDERS 30
#define ACCOUNTS 5
#define TOKENS 5
#endif

#include "macros.h"

struct Private {
    field254 orders[ORDERS*BITS_PER_ORDER];
    field254 pricesAndVolumes[(TOKENS*BITS_PER_DECIMAL)+(4*ORDERS*BITS_PER_DECIMAL)]; // price for each token, sell volume, buy volume, surplus (double width)
    field254 balances[ACCOUNTS*TOKENS*BITS_PER_DECIMAL];
};

struct In {
    field254 state;
    field254 surplus;
    field254 hashBatchInfo[2];
    field254 orderHash;
    field254 epsilon;
};

struct Out { field254 state; };