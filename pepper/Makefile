CXX = g++

# Make sure these are the same flags that pepper was built with
CXXFLAGS = -m64 -std=c++11 -DCURVE_ALT_BN128 -DBN_SUPPORT_SNARK -UBINARY_OUTPUT -DMONTGOMERY_OUTPUT -DNO_PROCPS -DUSE_ASM

LIBSNARK = $(PEPPER)/thirdparty/libsnark
PEPPER_BIN = $(PEPPER)/pepper/bin
IFLAGS = -Iext_gadget -I$(LIBSNARK)/ -I$(LIBSNARK)/depends/libff -I$(LIBSNARK)/depends/libfqfft
LDFLAGS = -L$(LIBSNARK)/build/libsnark -L$(LIBSNARK)/build/depends/libff/libff
LDFLAGS += -lsnark -lff -lgmp -lgmpxx

GADGET_SOURCES = ext_gadget/gadgets/common.cpp ext_gadget/gadgets/jubjub/*.cpp

TEST_LDFLAGS = -lgtest -lpthread
TEST_IFLAGS = -I./
TEST_BIN = test/bin/
FILENAMETESTS := $(shell find test/apps/ -name '*_test.cpp')

# We need to use ALT_BN_128 for all ethereum related work
change-curve: check-env
	sed -i 's/DCURVE_BN128/DCURVE_ALT_BN128/g' $(PEPPER)/pepper/Makefile

make-bin: check-env
	mkdir -p $(PEPPER_BIN)
	mkdir -p $(PEPPER)/pepper/prover_verifier_shared
	mkdir -p $(TEST_BIN)

# Convenience target to compile gadgets 0..n
gadgets: gadget0 gadget1

gadget0: ext_gadget/sha256_bridge.cpp check-env
	$(CXX) $(CXXFLAGS) $(IFLAGS) $<  $(GADGET_SOURCES) ext_gadget/common.cpp -o $(PEPPER_BIN)/gadget0 $(LDFLAGS)

gadget1: ext_gadget/pedersen_bridge.cpp check-env
	$(CXX) $(CXXFLAGS) $(IFLAGS) $< $(GADGET_SOURCES) ext_gadget/common.cpp -o $(PEPPER_BIN)/gadget1 $(LDFLAGS)

# Link source code of snark apps
link-apps: check-env
	ln -sf ~/dex-zksnarks/apps/* $(PEPPER)/pepper/apps/

# Link helper programs 0..n (e.g. for private input, bit decomposition, etc.)
link-exo: check-env
	ln -sf ~/dex-zksnarks/exo_compute/decompose_bits.py $(PEPPER_BIN)/exo1
	ln -sf ~/dex-zksnarks/exo_compute/private_input.py $(PEPPER_BIN)/exo0

# Target to run all tests (e.g. used by travis)
test: $(FILENAMETESTS)%o gadget-tests e2e-tests

e2e-tests: check-env
	test/e2e/hash_transform.sh

$(FILENAMETESTS)%o: $(FILENAMETESTS)	 
		$(CXX) $(CXXFLAGS) -DTEST $(TEST_IFLAGS) $(IFLAGS) $< -o $(TEST_BIN)/run_unit_tests $(TEST_LDFLAGS) $(LDFLAGS) 
		$(TEST_BIN)/run_unit_tests

unit-tests-iBV: test/apps/isBoolVerification_test.cpp
	$(CXX) $(CXXFLAGS) -DTEST $(TEST_IFLAGS) $(IFLAGS) $< -o $(TEST_BIN)/run_unit_tests $(TEST_LDFLAGS) $(LDFLAGS)
	$(TEST_BIN)/run_unit_tests
	

gadget-tests: test/gadgets/main.cpp
	$(CXX) $(CXXFLAGS) $(IFLAGS) -Iext_gadget/ $< ext_gadget/gadgets/*.cpp ext_gadget/gadgets/jubjub/*.cpp -o $(TEST_BIN)/run_gadget_tests $(LDFLAGS) $(TEST_LDFLAGS)
	$(TEST_BIN)/run_gadget_tests

# Make sure we have the environment set up
check-env:
ifndef PEPPER
  $(error $$PEPPER is undefined)
endif

initial: make-bin change-curve all

all: link-apps link-exo gadgets