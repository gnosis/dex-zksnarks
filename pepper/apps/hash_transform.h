#include <stdint.h>

#ifndef TEST
#define ORDERS 2
#endif

#ifdef TEST
typedef long int128;
#endif


struct Private { bool orders[ORDERS][253]; };
struct In { int128 shaHashL; int128 shaHashR; };
struct Out { uint32_t pedersenHash[2]; };

void compute(struct In *input, struct Out *output);
