#include "common.h"
#include "jubjub/pedersen_hash.hpp"
#include "jubjub/params.hpp"
#include "utils.hpp"

using namespace libsnark;
using namespace libff;

#define BUFLEN 10240

#define LEFT_INPUT_SIZE 255
#define RIGHT_INPUT_SIZE 255
#define OUTPUT_SIZE 2

uint64_t inputSize() {
    return LEFT_INPUT_SIZE + RIGHT_INPUT_SIZE;
}

uint64_t outputSize() {
    return OUTPUT_SIZE;
}

ethsnarks::jubjub::PedersenHash makeGadget(const char* assignment, protoboard<FieldT>& pb)
{
    const ethsnarks::jubjub::Params params;

    ethsnarks::VariableArrayT in;
    // The gadget expects input that is a multiple of three. Thus, we pad with 0s.
    assert(inputSize() % 3 == 0);
    in.allocate(pb, inputSize(), FMT("bridge", " scaler to multiply by"));

    ethsnarks::VariableT result_x = ethsnarks::make_variable(pb, FMT("bridge", "result_x"));
    ethsnarks::VariableT result_y = ethsnarks::make_variable(pb, FMT("bridge", "result_y"));

    ethsnarks::jubjub::PedersenHash f(pb, params, "dex.pedersen-hash", in, "gadget");
    f.generate_r1cs_constraints();

    pb.add_r1cs_constraint(
        ethsnarks::ConstraintT(result_x, 1, f.result_x()),
            FMT("bridge", "bridge.result_x=gadget.result_x"));
    pb.add_r1cs_constraint(
        ethsnarks::ConstraintT(result_y, 1, f.result_y()),
            FMT("bridge", "bridge.result_y=gadget.result_y"));

    if (assignment) {
        libff::bit_vector in_bv;
        for (int i = 0; i < inputSize(); i++) {
            in_bv.push_back(assignment[2*i] - '0');
        }
        in.fill_with_bits(pb, in_bv);
        f.generate_r1cs_witness();
        pb.val(result_x) = pb.val(f.result_x());
        pb.val(result_y) = pb.val(f.result_y());
        assert(pb.is_satisfied());
    }
    
    return f;
}

protoboard<FieldT> getProtoboard(const char* assignment) {
    ethsnarks::ppT::init_public_params();
    protoboard<FieldT> pb;
    makeGadget(assignment, pb);
    return pb;
}