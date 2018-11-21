#include <stdint.h>
#include "dex_common.h"

#define PEDERSEN_HASH_SIZE 508
#define SHA_HASH_SIZE 512

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
        bool pedersenInput[PEDERSEN_HASH_SIZE] = { 0 };
        copyBits(decomposed, 0, pedersenInput, 0, 254);
        uint32_t padding = 254 - chunkSize;
        copyBits(in, index, pedersenInput, 254+padding, chunkSize);

        ext_gadget(pedersenInput, result, 1);
    }
    return result->values[0];
}

/**
 * Hashes the given input of size `inputSize` by dividing it into windows of size 
 * `chunkSize` and chaining their SHA256 hashes together. It startes with 256 0-bits
 */
struct ShaHash { bool digest[256]; };
void hashSHA(bool *in, uint32_t inputSize, uint32_t chunkSize, int128 result[2]) {
    struct ShaHash shaOut[1] = { 0 };

    bool shaIn[SHA_HASH_SIZE] = { 0 };
    uint32_t index;
    for (index=0; index < inputSize; index+=chunkSize) {
        // sha hash
        copyBits(shaOut->digest, 0, shaIn, 0, 256);
        uint32_t padding = 256 - chunkSize;
        copyBits(in, index, shaIn, 256 + padding, chunkSize);
        ext_gadget(shaIn, shaOut, 0);
    }

    result[0] = sumBits(shaOut->digest, 128);
    result[1] = sumBits(shaOut->digest+128, 128);
}