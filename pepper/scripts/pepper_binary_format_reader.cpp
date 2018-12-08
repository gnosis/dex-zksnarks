#include <fstream>
#include <iostream>
#include <libsnark/common/default_types/r1cs_ppzksnark_pp.hpp>

#define PROGRESS_INTERVAL 1000000
#define PROGRESS_UNIT "M"

using namespace std;

typedef libff::Fr<libsnark::default_r1cs_ppzksnark_pp> FieldT;

int main(int argc, char **argv) 
{   
	if (argc != 3) {
		std::cout << "Usage: " << argv[0] << " <input> <output>\n";
		return 1;
	}
    libsnark::default_r1cs_ppzksnark_pp::init_public_params();

	ifstream source(argv[1], std::ios::binary);
    FILE *target = fopen(argv[2], "w");
    FieldT current;
    mpz_t r; mpz_init(r);
    size_t index = 0;

    // Field elements are seperated by space. 
    // Read a single element, then get one character (space) until EOF
    do {
        source >> current;
        current.as_bigint().to_mpz(r);
        gmp_fprintf(target, "%Zd ", r);
        index++;
        if (index % PROGRESS_INTERVAL == 0) {
            std::cout << index / PROGRESS_INTERVAL << PROGRESS_UNIT << " constraints \n";
        }

    } while (source.get() > 0);

    source.close();
    fclose(target);

    std::cout << index << " total constraints \n";
}
