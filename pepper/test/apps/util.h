#include <apps/hashing.h>
#include <boost/stacktrace.hpp>
#include <openssl/sha.h>
#include <pedersen_bridge.cpp>

#ifndef ORDERS
#define ORDERS 1
#endif

namespace pepper_overrides
{
    void charArrayToFieldArray(unsigned char *in, field254 *out, size_t length) {
        for (size_t index = 0; index < length; index++) {
            for (size_t bit = 0; bit < 8; bit++) {
                out[(index*8) + bit] = (in[index] >> (7 - bit)) % 2;
            }
        }
    }

    void fieldArrayToCharArray(field254 *in, unsigned char *out, size_t length) {
        for (size_t index = 0; index < length; index++) {
            out[index/8] += in[index].is_zero() ? 0 : int(pow(2, 7 - (index % 8)));
        }
    }

    void sha(field254 in[SHA_HASH_SIZE], field254 out[256]) {
        unsigned char charIn[SHA_HASH_SIZE/8] = {0};
        unsigned char charOut[32] = {0};
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        fieldArrayToCharArray(in, charIn, SHA_HASH_SIZE);
        SHA256_Update(&sha256, charIn, SHA_HASH_SIZE/8);
        SHA256_Final(charOut, &sha256);
        charArrayToFieldArray(charOut, out, 32);
    }

    void pedersen(field254 in[PEDERSEN_HASH_SIZE], field254 out[2]) {
        protoboard<FieldT> pb;
        char charIn[2*PEDERSEN_HASH_SIZE] = {0};
        for (size_t i = 0; i < PEDERSEN_HASH_SIZE; i++) {
            charIn[2*i] = in[i].is_zero() ? '0' : '1';
            charIn[2*i + 1] = ' ';
        }
        auto gadget = makeGadget(charIn, pb);
        out[0] = pb.val(gadget.result_x());
        out[1] = pb.val(gadget.result_y());
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
        case 1:
            pepper_overrides::decomposeBits(input[0][0],((Decomposed*) output)->bits, 0);
            break;
        default:
            ((Private*)output)[0] = privateInput;
            break;
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