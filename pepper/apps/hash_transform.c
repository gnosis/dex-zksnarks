#include <stdint.h>

#define ORDERS 2

struct In { uint32_t shaHash; };
struct Out { uint32_t pedersenHash[2]; };

struct Private { bool orders[ORDERS][253]; };
struct ShaHash { bool digest[256]; };
struct PedersenDecomposed { bool bits[254]; };
struct PedersenHash { uint32_t values[2]; }; /* [x,y] */

void compute(struct In *input, struct Out *output){
    // Read private input
    struct Private private = readPrivateInput();
    struct ShaHash shaHash[1];
    struct PedersenHash pedersenHash[1];

    bool shaIn[512];
    bool pedersenInput[508];

    uint32_t index;
    for (index=0; index < ORDERS; index++) {
        // sha hash
        copyBits(shaHash->digest, shaIn, 256);
        copyBits(private.orders[index], shaIn+259, 253);
        ext_gadget(shaIn, shaHash, 0);
        uint32_t result = sumBits(shaHash->digest, 256);

        // pedersen hash
        bool decomposed[254] = decomposeBits(pedersenHash->values[0]);
        copyBits(decomposed, pedersenInput, 254);
        copyBits(private.orders[index], pedersenInput+255, 253);
        ext_gadget(pedersenInput, pedersenHash, 1);
    }

    // assert final sha hash is equal with public input
    uint32_t result = sumBits(shaHash->digest, 256);
    assert_zero(result - input->shaHash);

    // Write final pederson hash to output
    output->pedersenHash[0] = pedersenHash->values[0];
    output->pedersenHash[1] = pedersenHash->values[1];
}

/**
 * Copies `lenght` bits from source to target
 */
void copyBits(bool *source, bool *target, uint32_t length) {
    uint32_t index;
    for (index = 0; index < length; index++) {
        target[index] = source[index];
    }
}

/**
 * sums `length` bits from `bits` to its integer representation  
 */
uint32_t sumBits(bool *bits, uint32_t length) {
    uint32_t result = 0;
    uint32_t pow = 1;
    uint32_t index = 0;
    for (index=0; index<length; index++) {
        result += bits[length-1-index] * pow;
        pow = pow * 2;
    }
    return result;
}

/**
 * Read private input from exo_compute (ID 0)
 */
struct Private readPrivateInput() {
    struct Private p[1];
    uint32_t lens[1] = {1};
    uint32_t *exo0_inputs[1] = {lens};
    exo_compute(exo0_inputs,lens,p,0);
    return p[0];
}

/**
 * Calling into external program to decompose bits.
 * Afterwards verify that bits sum up to number.
 */
bool* decomposeBits(uint32_t number) {
    struct PedersenDecomposed result[1];
    uint32_t lens[1] = {1};
    uint32_t input[1] = {number};
    uint32_t *exo1_inputs[1] = { input };
    exo_compute(exo1_inputs,lens,result,1);
    uint32_t sum = sumBits(result->bits, 254);
    assert_zero(sum - number);
    return result->bits;
}