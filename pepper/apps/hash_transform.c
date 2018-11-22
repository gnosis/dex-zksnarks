#include "hash_transform.h"
#include "dex_common.h"
#include "hashing.h"

void compute(struct In *input, struct Out *output){
    // Read private input
    struct Private pInput = readPrivateInput();
   
    int128 result[2];
    hashSHA(pInput.orders, ORDERS*253, 253, result);

    assert_zero(result[0] - input->shaHashL);
    assert_zero(result[1] - input->shaHashR);

    output->pedersenHash = hashPedersen(pInput.orders, ORDERS*253, 253);
}
