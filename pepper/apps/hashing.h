#include <stdint.h>
#include "dex_common.h"

/**
 * Hashes the given input of size `inputSize` by dividing it into windows of size 
 * `chunkSize` and chaining their pedersen hashes together and returns the x-coordinate
 * of the result. The starting point of the chain is the point at infinity (x = 0)
 *
 * E.g. hashPedersen([0,0,1,1], 4, 2) == hash(hash(O, [0,0]), [1,1])
 */
struct PedersenHash { uint32_t values[2]; }; /* resulting Point(x,y) */
uint32_t hashPedersen(bool *in, uint32_t inputSize, uint32_t chunkSize) {
    struct PedersenHash result[1] = { 0 };
    uint32_t index;
    for (index = 0; index < inputSize; index += chunkSize) {
        // Decompose x-coordinate from previous result to binary
        bool decomposed[254] = { 0 };
        decomposeBits(result->values[0], decomposed);

        // Fill pedersen input, left with previous result
        bool pedersenInput[508] = { 0 };
        copyBits(decomposed, 0, pedersenInput, 0, 254);
        uint32_t padding = 254 - chunkSize;
        copyBits(in, index, pedersenInput, 254+padding, chunkSize);

        ext_gadget(pedersenInput, result, 1);
    }
    return result->values[0];
}