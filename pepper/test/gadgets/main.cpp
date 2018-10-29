#include <gtest/gtest.h>
#include <libsnark/common/default_types/r1cs_ppzksnark_pp.hpp>
typedef libff::Fr<libsnark::default_r1cs_ppzksnark_pp> FieldT;

#include "sha256_test.cpp"
#include "pedersen_test.cpp"

int main(int argc, char **argv) {
    testing::FLAGS_gtest_death_test_style = "threadsafe";
	libsnark::default_r1cs_ppzksnark_pp::init_public_params();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}