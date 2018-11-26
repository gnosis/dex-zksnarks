#include "hash_transform.h"
#include "dex_common.h"

struct ShaHash { field254 digest[256]; };
struct PedersenHash { field254 values[2]; }; /* [x,y] */

void compute(struct In *input, struct Out *output){
    // Read private input
    struct Private pInput = readPrivateInput();
    struct ShaHash shaHash[1] = { 0 };
    struct PedersenHash pedersenHash[1] = { 0 };

    field254 shaIn[512] = { 0 };
    field254 pedersenInput[508] = { 0 };

    uint32_t index;
    for (index=0; index < ORDERS; index++) {
        // sha hash
        copyBits(shaHash->digest, shaIn, 256);
        copyBits(pInput.orders[index], shaIn+259, 253);
        ext_gadget(shaIn, shaHash, 0);

        // pedersen hash
        field254 decomposed[254] = { 0 };
        decomposeBits(pedersenHash->values[0], decomposed);
        copyBits(decomposed, pedersenInput, 254);
        copyBits(pInput.orders[index], pedersenInput+255, 253);
        ext_gadget(pedersenInput, pedersenHash, 1); 
    };
    
    // assert final sha hash is equal with public input
    field254 resultL = sumBits(shaHash->digest, 128);
    field254 resultR = sumBits(shaHash->digest+128, 128);
   
    assert_zero(resultL - input->shaHashL);
    assert_zero(resultR - input->shaHashR);

    // Write final pederson hash to output
    output->pedersenHash[0] = pedersenHash->values[0];
    output->pedersenHash[1] = pedersenHash->values[1];
}