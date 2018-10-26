#include <stdint.h>

#ifndef TEST
#define ORDERS 2
#endif

struct Private { bool orders[ORDERS][253]; };
struct In { uint32_t shaHash; };
struct Out { uint32_t pedersenHash[2]; };

void compute(struct In *input, struct Out *output);