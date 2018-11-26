#include "hash_transform.h"
#include "dex_common.h"
#include "hashing.h"

void compute(struct In *input, struct Out *output){
    // Read private input
    struct Private pInput = readPrivateInput();   
    struct ShaResult result = hashSHA(pInput.orders, ORDERS*253, 253);

    assert_zero(result.left - input->shaHashL);
    assert_zero(result.right - input->shaHashR);

    output->pedersenHash = hashPedersen(pInput.orders, ORDERS*253, 253);
}
