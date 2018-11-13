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

using namespace jubjub;

pedersen_commitment::pedersen_commitment(
    ProtoboardT &pb,
    const VariableArrayT &left, const VariableArrayT &right,
    const VariableT &commitment_x, const VariableT &commitment_y
) :
    GadgetT(pb, " pedersen_commitment"),
    m(left), r(right),
    commitment_x(commitment_x),
    commitment_y(commitment_y),
    base_x("17777552123799933955779906779655732241715742912184938656739573121738514868268"),
    base_y("2626589144620713026669568689430873010625803728049924121243784502389097019475"),
    H_x("16540640123574156134436876038791482806971768689494387082833631921987005038935"),
    H_y("20819045374670962167435360035096875258406992893633759881276124905556507972311")
{
    //Curve parameters
    a.allocate(pb, "a");
    d.allocate(pb, "d");
    pb.val(a) = FieldT("168700");
    pb.val(d) = FieldT("168696");

    // base * m
    jubjub_pointMultiplication_lhs.reset( new fixed_base_mul (pb, params, base_x, base_y, m, " lhs mul "));
    // h*r
    jubjub_pointMultiplication_rhs.reset( new fixed_base_mul (pb, params, H_x, H_y, r, "rhs mul "));
    jubjub_pointAddition.reset( new pointAddition (pb, a, d, jubjub_pointMultiplication_rhs->result_x(), jubjub_pointMultiplication_rhs->result_y(), jubjub_pointMultiplication_lhs->result_x(), jubjub_pointMultiplication_lhs->result_y(), commitment_x, commitment_y , "rhs addition"));
}

void pedersen_commitment::generate_r1cs_constraints()
{
    jubjub_pointMultiplication_lhs->generate_r1cs_constraints();
    jubjub_pointMultiplication_rhs->generate_r1cs_constraints();
    jubjub_pointAddition->generate_r1cs_constraints();
}


void  pedersen_commitment::generate_r1cs_witness()
{
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
