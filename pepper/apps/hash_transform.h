#include <stdint.h>

#define ORDERS 2

struct Private { field orders[ORDERS][253]; };

struct In { field shaHashL; field shaHashR; };
struct Out { field pedersenHash[2]; };

void compute(struct In *input, struct Out *output);
