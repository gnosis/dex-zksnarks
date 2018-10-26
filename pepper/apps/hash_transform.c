#include "hash_transform.h"
#include "dex_common.h"

struct ShaHash { bool digest[256]; };
struct PedersenHash { uint32_t values[2]; }; /* [x,y] */

void compute(struct In *input, struct Out *output){
    // Read private input
    struct Private pInput = readPrivateInput();
    struct ShaHash shaHash[1] = { 0 };
    struct PedersenHash pedersenHash[1] = { 0 };

    bool shaIn[512] = { 0 };
    bool pedersenInput[508] = { 0 };

    uint32_t index;
    for (index=0; index < ORDERS; index++) {
        // sha hash
        copyBits(shaHash->digest, shaIn, 256);
        copyBits(pInput.orders[index], shaIn+259, 253);
        ext_gadget(shaIn, shaHash, 0);
        uint32_t result = sumBits(shaHash->digest, 256);

        // pedersen hash
        bool decomposed[254] = { 0 };
        decomposeBits(pedersenHash->values[0], decomposed);
        copyBits(decomposed, pedersenInput, 254);
        copyBits(pInput.orders[index], pedersenInput+255, 253);
        ext_gadget(pedersenInput, pedersenHash, 1);
    }

    // assert final sha hash is equal with public input
    uint32_t result = sumBits(shaHash->digest, 256);
    assert_zero(result - input->shaHash);

    // Write final pederson hash to output
    output->pedersenHash[0] = pedersenHash->values[0];
    output->pedersenHash[1] = pedersenHash->values[1];
}