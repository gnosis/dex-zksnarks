#ifndef ORDERS
#define ORDERS 1
#endif

namespace pepper_overrides
{
    /**
     * value will be 2**<#sha_calls>
     */
    size_t shaCount = 0;
    void sha(bool in[SHA_HASH_SIZE], bool out[256]) {
        out[255-shaCount] = 1;
        shaCount++;
    }

    /**
     * x value will be the number of times this function has been called
     */
    size_t pedersenCount = 1;
    void pedersen(bool in[PEDERSEN_HASH_SIZE], uint32_t out[2]) {
        out[0] = pedersenCount;
        pedersenCount++;
    }

    void privateInput(void* privateInput) {
        for (size_t order = 0; order < ORDERS; order++) {
            for (size_t bit = 0; bit < 253; bit ++) {
                ((bool*)privateInput)[(order*253) + bit] = 1;
            }
        }
    }

    void decomposeBits(uint32_t number, bool bits[254]) {
        int index = 0;
        while (number > 0) {
            bits[253 - index] = number % 2;
            number = number >> 1;
        }
    }
} // pepper_overrides

#ifndef EXT_GADGET_OVERRIDE
void ext_gadget(void* in, void* out, uint32_t gadget) {
    switch(gadget) {
        case 0:
            pepper_overrides::sha((bool*) in, (bool*) out);
            break;
        case 1:
            pepper_overrides::pedersen((bool*) in, (uint32_t *) out);
            break;
        default:
            FAIL();
    }
}
#endif

#ifndef EXO_COMPUT_OVERRIDE
void exo_compute(uint32_t** input, uint32_t* length, void* output, uint32_t exo) {
    switch(exo) {
        case 0:
            pepper_overrides::privateInput(output);
            break;
        case 1:
            pepper_overrides::decomposeBits(input[0][0],((Decomposed*) output)->bits);
            break;
        default:
            FAIL();
    }
}
#endif

#ifndef ASSERT_ZERO_OVERRIDE
void assert_zero(uint32_t v) {
    assert(0 == v);
}
#endif