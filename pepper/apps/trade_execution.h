#include <stdint.h>

#ifndef TEST
#define ORDERS 30
#define ACCOUNTS 10
#define TOKENS 5
#endif

struct Private {
    field254 orders[ORDERS*253];
    field254 pricesAndVolumes[(TOKENS*100)+(2*ORDERS*100)];
    field254 balances[ACCOUNTS*TOKENS*100];
};

struct In {
    field254 state;
    field254 welfare;
    field254 hashBatchInfo;
    field254 orderHash;
};

struct Out { field254 state; };