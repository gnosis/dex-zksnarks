#include "hash_transform.h"
#include "dex_common.h"
#include "hashing.h"

struct ShaHash { bool digest[256]; };

void compute(struct In *input, struct Out *output){
    // Read private input
    struct Private pInput = readPrivateInput();
    struct ShaHash shaHash[1] = { 0 };

    bool shaIn[512] = { 0 };
    uint32_t index;
    for (index=0; index < ORDERS*253; index+=253) {
        // sha hash
        copyBits(shaHash->digest, 0, shaIn, 0, 256);
        copyBits(pInput.orders, index, shaIn, 259, 253);
        ext_gadget(shaIn, shaHash, 0);
        uint32_t result = sumBits(shaHash->digest, 256);
    }

    // assert final sha hash is equal with public input
    int128 resultL = sumBits(shaHash->digest, 128);
    int128 resultR = sumBits(shaHash->digest+128, 128);
   
    assert_zero(resultL - input->shaHashL);
    assert_zero(resultR - input->shaHashR);

    output->pedersenHash = hashPedersen(pInput.orders, ORDERS*253, 253);
}