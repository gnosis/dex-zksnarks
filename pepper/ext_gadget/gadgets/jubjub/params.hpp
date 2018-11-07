#ifndef JUBJUB_PARAMS_HPP_
#define JUBJUB_PARAMS_HPP_

#include <gadgets/common.hpp>

namespace ethsnarks {

namespace jubjub {


class Params {
public:
    // Edwards parameters
    const FieldT a;
    const FieldT d;

    // Montgomery parameters
    const FieldT A;

    Params() :
        a("168700"),
        d("168696"),
        A("168698")
    {}
};


// namespace jubjub
}

// namespace ethsnarks
}

// JUBJUB_PARAMS_HPP_
#endif
