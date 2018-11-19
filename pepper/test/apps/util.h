namespace pepper_overrides
{
    /**
     * value will be 2**<#sha_calls>
     */
    size_t shaCount = 0;
    void sha(field in[512], field out[256]) {
        out[255-shaCount] = 1;
        shaCount++;
    }

    /**
     * x value will be the number of times this function has been called
     */
    size_t pedersenCount = 1;
    void pedersen(field in[508], field out[2]) {
        out[0] = pedersenCount;
        pedersenCount++;
    }

    void privateInput(void* privateInput) {
        for (size_t order = 0; order < ORDERS; order++) {
            for (size_t bit = 0; bit < 253; bit ++) {
                ((field*)privateInput)[(order*253) + bit] = field::one();
            }
        }
    }

    void decomposeBits(field number, field bits[254]) {
        int index = 0;
        unsigned long nr = number.as_ulong();
        while (nr > 0) {
            if(nr%2==0)
                bits[253 - index] = field::zero();
            else
                bits[253 - index] = field::one();
            nr = nr/2;
        }
    }
} // pepper_overrides

void ext_gadget(void* in, void* out, uint32_t gadget) {
    switch(gadget) {
        case 0:
            pepper_overrides::sha((field*) in, (field*) out);
            break;
        case 1:
            pepper_overrides::pedersen((field*) in, (field *) out);
            break;
        default:
            FAIL();
    }
}

void exo_compute(field** input, uint32_t* length, void* output, uint32_t exo) {
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

void assert_zero(field v) {
    assert(field::zero() == v);
}