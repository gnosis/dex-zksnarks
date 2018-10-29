#include <gadgets/jubjub/pedersen_commitment.hpp>

struct Result {
	libff::bigint<FieldT::num_limbs> x;
	libff::bigint<FieldT::num_limbs> y;
};

void compute(const libff::bit_vector& left_bv, const libff::bit_vector& right_bv, Result* result) {
	libsnark::protoboard<FieldT> pb;
	
	libsnark::pb_variable_array<FieldT> left;
    left.allocate(pb, 254, FMT("annotation_prefix", " scaler to multiply by"));
    
	libsnark::pb_variable_array<FieldT> right;
    right.allocate(pb, 254, FMT("annotation_prefix", " scaler to multiply by"));

	libsnark::pb_variable<FieldT> commitment_x;
    libsnark::pb_variable<FieldT> commitment_y;
    commitment_x.allocate(pb, "r_x");
    commitment_y.allocate(pb, "r_y");

    ethsnarks::pedersen_commitment f(pb,left,right,commitment_x,commitment_y);
    f.generate_r1cs_constraints();

	left.fill_with_bits(pb, left_bv);
	right.fill_with_bits(pb, right_bv);
    
	f.generate_r1cs_witness();
	ASSERT_TRUE(pb.is_satisfied());

	libsnark::r1cs_auxiliary_input<FieldT> values = pb.full_variable_assignment();
	result->x = values[508].as_bigint();
	result->y = values[509].as_bigint();
}

TEST(PEDERSENTEST, TestZeroHash) {
	const bool input_array[254] = { 0 };
	libff::bit_vector left_bv(input_array, input_array + 254);
	libff::bit_vector right_bv(input_array, input_array + 254);

	struct Result result;
	compute(left_bv, right_bv, &result);

	ASSERT_EQ(result.x, libff::bigint<result.x.N>("0"));
	ASSERT_EQ(result.y, libff::bigint<result.x.N>("1"));
}

TEST(PEDERSENTEST, TestOneHash) {
	bool input_array[254] = { 0 };
	std::fill_n(input_array, 254, 1);
	libff::bit_vector left_bv(input_array, input_array + 254);
	libff::bit_vector right_bv(input_array, input_array + 254);

	struct Result result;
	compute(left_bv, right_bv, &result);

	ASSERT_EQ(result.x, libff::bigint<result.x.N>("18271182124286413948329422809975597182599531599794801990372568483492066594218"));
	ASSERT_EQ(result.y, libff::bigint<result.x.N>("17949587381215011496753821226245250831452811423551646685485316270001005134403"));
}