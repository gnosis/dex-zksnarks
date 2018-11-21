#include <stdint.h>

#define ORDERS 2

struct Private { bool orders[ORDERS * 253]; };
struct In { int128 shaHashL; int128 shaHashR; };
struct Out { uint32_t pedersenHash; };

void compute(struct In *input, struct Out *output);
