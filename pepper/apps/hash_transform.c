#include "hash_transform.h"
#include "dex_common.h"

#ifdef TEST
typedef long int128;
#endif

struct ShaHash { bool digest[256]; };
struct PedersenHash { uint32_t values[2]; }; /* [x,y] */

void compute(struct In *input, struct Out *output){
    // Read private input
    struct Private pInput = readPrivateInput();
    //printf("LHS of hash input %Zd and RHS of hash input %Zd ", input->shaHashL, input->shaHashR);

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

        // pedersen hash
        bool decomposed[254] = { 0 };
        decomposeBits(pedersenHash->values[0], decomposed);
        copyBits(decomposed, pedersenInput, 254);
        copyBits(pInput.orders[index], pedersenInput+255, 253);
        ext_gadget(pedersenInput, pedersenHash, 1); 
    };
    
    // assert final sha hash is equal with public input
    int128 resultL = sumBits(shaHash->digest, 128);
    int128 resultR = sumBits(shaHash->digest+128, 128);
    //printf("Results: LHS %Zd and RHS %Zd ", resultL, resultR);

    assert_zero(resultL - input->shaHashL);
    assert_zero(resultR - input->shaHashR);

    // Write final pederson hash to output
    output->pedersenHash[0] = pedersenHash->values[0];
    output->pedersenHash[1] = pedersenHash->values[1];
}