#ifndef ORDERS
#define ORDERS 1
#endif

namespace pepper_overrides
{
    /**
     * value will be 2**<#sha_calls>
     */
    size_t shaCount = 0;
    void sha(field254 in[SHA_HASH_SIZE], field254 out[256]) {
        out[255-shaCount] = 1;
        shaCount++;
    }

    /**
     * x value will be the number of times this function has been called
     */
    size_t pedersenCount = 1;
    void pedersen(field254 in[PEDERSEN_HASH_SIZE], field254 out[2]) {

        out[0] = pedersenCount;
        pedersenCount++;
    }

    void privateInput(void* privateInput) {
        for (size_t order = 0; order < ORDERS; order++) {
            for (size_t bit = 0; bit < 253; bit ++) {
                ((field254*)privateInput)[(order*253) + bit] = field254::one();
            }
        }
    }

    void decomposeBits(field254 number, field254 bits[254]) {
        int index = 0;
        unsigned long nr = number.as_ulong();
        while (nr > 0) {
            if(nr%2==0)
                bits[253 - index] = field254::zero();
            else
                bits[253 - index] = field254::one();
            nr = nr/2;
        }
    }
} // pepper_overrides

#ifndef EXT_GADGET_OVERRIDE
void ext_gadget(void* in, void* out, uint32_t gadget) {
    switch(gadget) {
        case 0:
            pepper_overrides::sha((field254*) in, (field254*) out);
            break;
        case 1:
            pepper_overrides::pedersen((field254*) in, (field254 *) out);
            break;
        default:
            FAIL();
    }
}
#endif

#ifndef EXO_COMPUT_OVERRIDE
void exo_compute(field254** input, uint32_t* length, void* output, uint32_t exo) {
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
void assert_zero(field254 v) {
    assert(field254::zero() == v);
}
#endif