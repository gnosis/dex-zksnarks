#include <iostream>

#include <gadgets/sha256_full.cpp>

void compute(const libff::bit_vector& input_bv, mpz_t* result) {
	libsnark::protoboard<FieldT> pb;

	libsnark::block_variable<FieldT> input(pb, 512, "input");
    libsnark::digest_variable<FieldT> output(pb, 256, "output");

    ethsnarks::sha256_full_gadget_512 f(pb, input, output, "f");
    f.generate_r1cs_constraints();

	input.generate_r1cs_witness(input_bv);
	f.generate_r1cs_witness();
	ASSERT_TRUE(pb.is_satisfied());

	// Check result matches expected	
	libsnark::r1cs_auxiliary_input<FieldT> values = pb.full_variable_assignment();
	std::stringstream output_bits;
	for (size_t i = 0; i < 256; i++) {
		output_bits << values[512+i].as_bigint();
	}
	mpz_set_str(*result, output_bits.str().c_str(), 2);
}

TEST(SHA256TEST, TestZeroHash) {
	const bool input_array[512] = { 0 };
	libff::bit_vector input_bv(input_array, input_array + 512);

	mpz_t actual;
	mpz_init(actual);
	compute(input_bv, &actual);

	mpz_t expected;
	mpz_init(expected);
	mpz_set_str(expected, "f5a5fd42d16a20302798ef6ed309979b43003d2320d9f0e8ea9831a92759fb4b", 16);
	
	ASSERT_EQ(mpz_cmp(actual, expected), 0);
}

TEST(SHA256TEST, TestOneHash) {
	bool input_array[512] = { 0 };
	std::fill_n(input_array, 512, 1);
	libff::bit_vector input_bv(input_array, input_array + 512);

	mpz_t actual;
	mpz_init(actual);
	compute(input_bv, &actual);

	mpz_t expected;
	mpz_init(expected);
	mpz_set_str(expected, "8667e718294e9e0df1d30600ba3eeb201f764aad2dad72748643e4a285e1d1f7", 16);

	ASSERT_EQ(mpz_cmp(actual, expected), 0);
}