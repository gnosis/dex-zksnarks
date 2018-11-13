#include <stdint.h>
#include "dex_common.h"


struct In { int128 shaHashL; int128 shaHashR; };
struct Out { uint32_t pedersenHash[2]; };

void compute(struct In *input, struct Out *output);
