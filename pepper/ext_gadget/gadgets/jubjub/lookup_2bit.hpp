#ifndef ETHSNARKS_LOOKUP_2BIT_HPP_
#define ETHSNARKS_LOOKUP_2BIT_HPP_

// Copyright (c) 2018 HarryR
// License: LGPL-3.0+

#include <gadgets/common.hpp>

namespace ethsnarks {

/**
* Two-bit window lookup table using one constraint
*/
class lookup_2bit_gadget : public GadgetT
{
public:
    const std::vector<FieldT> c;
    const VariableArrayT b;
    VariableT r;

    lookup_2bit_gadget(
        ProtoboardT &in_pb,
        const std::vector<FieldT> in_constants,
        const VariableArrayT in_bits,
        const std::string& annotation_prefix
    );

    const VariableT& result();

    void generate_r1cs_constraints();

    void generate_r1cs_witness ();
};

// namespace ethsnarks
}

#endif
