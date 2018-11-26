#include <stdint.h>

#define ORDERS 2

struct Private { field254 orders[ORDERS][253]; };
struct In { field254 shaHashL; field254 shaHashR; };
struct Out { field254 pedersenHash[2]; };

void compute(struct In *input, struct Out *output);
