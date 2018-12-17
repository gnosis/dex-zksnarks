#include <boost/stacktrace.hpp>

#ifndef ORDERS
#define ORDERS 1
#endif

namespace pepper_overrides
{
    /**
     * Dummy pedersen implementation: XOR lhs and rhs bits
     */
    void sha(field254 in[SHA_HASH_SIZE], field254 out[256]) {
        for (size_t index = 0; index < 256; index++) {
            out[index] += in[index].is_zero() ^ in[index+256].is_zero();
        }
    }

    /**
     * Dummy pedersen implementation: Sum of all inputs
     */
    void pedersen(field254 in[PEDERSEN_HASH_SIZE], field254 out[2]) {
        for (size_t index = 0; index < PEDERSEN_HASH_SIZE; index++) {
            out[0] += in[index];
        }
    }

    void decomposeBits(field254 number, field254* bits, uint32_t offset) {
        int index = 0;
        unsigned long nr = number.as_ulong();
        while (nr > 0) {
            if(nr%2==0)
                bits[offset + 253 - index] = field254::zero();
            else
                bits[offset + 253 - index] = field254::one();
            nr = nr/2;
            index++;
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
struct Private privateInput;
void exo_compute(field254** input, uint32_t* length, void* output, uint32_t exo) {
    switch(exo) {
        case 0:
            ((Private*)output)[0] = privateInput;
            break;
        case 1:
            pepper_overrides::decomposeBits(input[0][0],((Decomposed*) output)->bits, 0);
            break;
        default:
            FAIL();
    }
}
#endif

#ifndef ASSERT_ZERO_OVERRIDE
bool DISABLE_STACKTRACE = false;
void assert_zero(field254 v) {
    if (field254::zero() != v && !DISABLE_STACKTRACE) {
        std::cout << boost::stacktrace::stacktrace();
    }
    assert(field254::zero() == v);
}
#endif