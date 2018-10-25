/*    
    copyright 2018 to the baby_jubjub_ecc Authors

    This file is part of baby_jubjub_ecc.

    baby_jubjub_ecc is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    baby_jubjub_ecc is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with baby_jubjub_ecc.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "pedersen_commitment.hpp"

namespace ethsnarks
{


pedersen_commitment::pedersen_commitment(
    ProtoboardT &pb,
    const VariableArrayT &left, const VariableArrayT &right,
    const VariableT &commitment_x, const VariableT &commitment_y
) :
    GadgetT(pb, " pedersen_commitment"),
    m(left), r(right),
    commitment_x(commitment_x),
    commitment_y(commitment_y)
{
    //Curve parameters
    a.allocate(pb, "a");
    d.allocate(pb, "d");
    pb.val(a) = FieldT("168700");
    pb.val(d) = FieldT("168696");

    // Generator point
    base_x.allocate(pb, "base x");
    base_y.allocate(pb, "base y");
    pb.val(base_x) = FieldT("17777552123799933955779906779655732241715742912184938656739573121738514868268");
    pb.val(base_y) = FieldT("2626589144620713026669568689430873010625803728049924121243784502389097019475");

    // H
    H_x.allocate(pb, "h_x");
    H_y.allocate(pb, "h_y");
    pb.val(H_x) = FieldT("16540640123574156134436876038791482806971768689494387082833631921987005038935");
    pb.val(H_y) = FieldT("20819045374670962167435360035096875258406992893633759881276124905556507972311");

    lhs_x.allocate(pb, 254,  FMT("lhs x", " pedersen_commitment"));
    lhs_y.allocate(pb, 254, FMT("lhs y", " pedersen_commitment"));
    rhs_x.allocate(pb,254, FMT( "rhs mul x" , " pedersen_commitment" ));
    rhs_y.allocate(pb,254, FMT( "rhs mul y ", " pedersen_commitment"));

    // make sure both points are on the twisted edwards cruve
    jubjub_isOnCurve1.reset( new isOnCurve (pb, base_x,base_y, a, d, "Confirm x, y is on the twiseted edwards curve"));
    jubjub_isOnCurve2.reset( new isOnCurve (pb, H_x, H_y, a, d, "Confirm x, y is on the twiseted edwards curve"));

    // base * m
    jubjub_pointMultiplication_lhs.reset( new pointMultiplication (pb, a, d, base_x, base_y, m, lhs_x, lhs_y, " lhs check ", 254));
    // h*r
    jubjub_pointMultiplication_rhs.reset( new pointMultiplication (pb, a, d, H_x, H_y, r, rhs_x, rhs_y, "rhs mul ", 254));
    jubjub_pointAddition.reset( new pointAddition (pb, a, d, rhs_x[253], rhs_y[253] , lhs_x[253] , lhs_y[253], commitment_x, commitment_y , "rhs addition"));
}

void pedersen_commitment::generate_r1cs_constraints()
{
    // not sure if we need to check pub key and r 
    // are on the curve. But doing it here for defense
    // in depth
    jubjub_isOnCurve1->generate_r1cs_constraints();
    jubjub_isOnCurve2->generate_r1cs_constraints();

    jubjub_pointMultiplication_lhs->generate_r1cs_constraints();
    jubjub_pointMultiplication_rhs->generate_r1cs_constraints();
    jubjub_pointAddition->generate_r1cs_constraints();
}


void  pedersen_commitment::generate_r1cs_witness()
{
    jubjub_isOnCurve1->generate_r1cs_witness();
    jubjub_isOnCurve2->generate_r1cs_witness();
    jubjub_pointMultiplication_lhs->generate_r1cs_witness();
    jubjub_pointMultiplication_rhs->generate_r1cs_witness();
    jubjub_pointAddition->generate_r1cs_witness();


    //debug
    /*
    std::cout <<  this->pb.lc_val(lhs_x[253]) << " " <<  this->pb.lc_val(rhs_x) << " "<< std::endl; // <<  this->pb.lc_val(S) << " " <<  this->pb.lc_val(H) ;
    for (uint i = 0 ; i < 254; i++) { 
        std::cout << i << " i  " << this->pb.lc_val(S[i]) << std::endl;
    }*/
}

// namespace ethsnarks
}
