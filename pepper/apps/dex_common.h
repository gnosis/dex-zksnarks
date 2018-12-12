#pragma once

/**
 * Copies `length` bits from source to target
 */
void copyBits(field254* source, uint32_t source_offset, field254* target, uint32_t target_offset, uint32_t length) {
    uint32_t index;
    for (index = 0; index < length; index++) {
        target[target_offset+index] = source[source_offset+index];
    }
}

/**
 * sums `length` bits from `bits` to its integer representation  
 */
field254 sumBits(field254 *bits, uint32_t offset, uint32_t length) {
    field254 result = 0;
    field254 pow = 1;
    uint32_t index = 0;
    for (index=0; index<length; index++) {
        result += bits[offset+length-1-index] * pow;
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
    field254 ones[1] = {1};
    field254 *exo0_inputs[1] = {ones};
    exo_compute(exo0_inputs,lens,p,0);
    return p[0];
}

/**
 * checks that all numbers in the array are of boolean type  
 */
void isBoolVerification(field254 *bits, uint32_t length) {
    uint32_t index = 0;
    for (index=0; index<length; index++) {
        assert_zero(bits[index] * (bits[index] - 1));
    }
}

/**
 * Calling into external program to decompose bits.
 * Afterwards verify that bits sum up to number.
 */
struct Decomposed { field254 bits[254]; };
void decomposeBits(field254 number, field254* bits, uint32_t offset) {
    struct Decomposed result[1] = { 0 };
    uint32_t lens[1] = {1};
    field254 input[1] = {number};
    field254 *exo1_inputs[1] = { input };
    exo_compute(exo1_inputs,lens,result,1);
    isBoolVerification(result->bits, 254);
    field254 sum = sumBits(result->bits, 0, 254);
    assert_zero(sum - number);
    copyBits(result->bits, 0, bits, offset, 254);
}