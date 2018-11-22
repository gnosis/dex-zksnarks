#pragma once

#ifdef PEPPER
// Pepper (C) doesn't know bool
typedef unsigned char bool;
#endif

/**
 * Copies `length` bits from source to target
 */
void copyBits(bool* source, uint32_t source_offset, bool* target, uint32_t target_offset, uint32_t length) {
    uint32_t index;
    for (index = 0; index < length; index++) {
        target[target_offset+index] = source[source_offset+index];
    }
}

/**
 * sums `length` bits from `bits` to its integer representation  
 */
int128 sumBits(bool *bits, uint32_t length) {
    int128 result = 0;
    int128 pow = 1;
    int128 index = 0;
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
struct Decomposed { bool bits[254]; };
void decomposeBits(uint32_t number, bool* bits) {
    struct Decomposed result[1] = { 0 };
    uint32_t lens[1] = {1};
    uint32_t input[1] = {number};
    uint32_t *exo1_inputs[1] = { input };
    exo_compute(exo1_inputs,lens,result,1);
    uint32_t sum = sumBits(result->bits, 254);
    assert_zero(sum - number);
	copyBits(result->bits, 0, bits, 0, 254);
}