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
struct Private readPrivateInput(uint32_t id) {
    struct Private p[1];
    uint32_t lens[1] = {1};
    field254 ones[1] = {1};
    field254 *exo0_inputs[1] = {ones};
    exo_compute(exo0_inputs,lens,p,id);
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
void decomposeBits(field254 number, field254* bits, uint32_t offset, uint32_t size) {
    struct Decomposed result[1] = { 0 };
    uint32_t lens[1] = {1};
    field254 input[1] = {number};
    field254 *exo1_inputs[1] = { input };
    exo_compute(exo1_inputs,lens,result,1);
    isBoolVerification(result->bits, 254);
    field254 sum = sumBits(result->bits, 0, 254);
    assert_zero(sum - number);
    copyBits(result->bits, 254-size, bits, offset, size);
}

uint32_t fieldToInt(field254 field) {
#ifndef BIGINT
    return field;
#else
    return field.as_ulong();
#endif
}

field254 maxPositiveFieldBits[254] = {0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool isNegative(field254 field) {
#ifndef BIGINT
    field254 fieldBits[254] = {0};
    decomposeBits(field, fieldBits, 0, 254);
    uint32_t index;
    bool result, found = 0;
    for(index = 0; index < 254; index++) {
        if (found == 1) {
            // we have found the different bit
        } else if(maxPositiveFieldBits[index] - fieldBits[index] == 1) {
            // maxPositiveField > field -> not negative
            found = 1;
        } else if (maxPositiveFieldBits[index] - fieldBits[index] != 0) {
            // maxPositiveField < field -> negative
            result = 1;
            found = 1;
        }
    }
    //maxPositiveField == field -> not negative
    return result;
#else
    mpz_t max_signed, r;
    mpz_init(r); mpz_init_set_str(max_signed, "10944121435919637611123202872628637544274182200208017171849102093287904247808", 10);
    field.as_bigint().to_mpz(r);
    return mpz_cmp(r, max_signed) > 0;
#endif
}