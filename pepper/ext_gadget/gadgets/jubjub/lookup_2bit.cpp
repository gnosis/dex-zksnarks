// Copyright (c) 2018 HarryR
// License: LGPL-3.0+

#include "lookup_2bit.hpp"

#include <gadgets/common.hpp>

namespace ethsnarks {
	
lookup_2bit_gadget::lookup_2bit_gadget(
	ProtoboardT &in_pb,
	const std::vector<FieldT> in_constants,
	const VariableArrayT in_bits,
	const std::string& annotation_prefix
) :
	GadgetT(in_pb, annotation_prefix),
	c(in_constants),
	b(in_bits),
	r(make_variable(in_pb, FMT(this->annotation_prefix, ".r")))
{
	assert( in_constants.size() == 4 );
}

const VariableT& lookup_2bit_gadget::result() {
	return r;
}

void lookup_2bit_gadget::generate_r1cs_constraints() {
    this->pb.add_r1cs_constraint(
        ConstraintT(
            { (b[1]*c[3]) - (b[1]*c[2]) - (b[1]*c[1]) + c[1] + (c[0]*b[1]) - c[0] },
            b[0],
            { r - (b[1]*c[2]) + (c[0]*b[1]) - c[0] }
            ),
            FMT(this->annotation_prefix, ".result"));
}

void lookup_2bit_gadget::generate_r1cs_witness () {
    auto i = b.get_field_element_from_bits(this->pb).as_ulong();
    this->pb.val(r) = c[i];
}

// namespace ethsnarks
}
