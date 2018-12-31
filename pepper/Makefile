CXX = g++

# Make sure these are the same flags that pepper was built with
CXXFLAGS = -m64 -std=c++11 -DCURVE_ALT_BN128 -DBN_SUPPORT_SNARK -UBINARY_OUTPUT -DMONTGOMERY_OUTPUT -DNO_PROCPS -DUSE_ASM

LIBSNARK = $(PEPPER)/thirdparty/libsnark
PEPPER_BIN = $(PEPPER)/pepper/bin
IFLAGS = -Iext_gadget -I$(LIBSNARK)/ -I$(LIBSNARK)/depends/libff -I$(LIBSNARK)/depends/libfqfft
LDFLAGS = -L$(LIBSNARK)/build/libsnark -L$(LIBSNARK)/build/depends/libff/libff
LDFLAGS += -lsnark -lff -lgmp -lgmpxx

TEST_LDFLAGS = -lgtest -lpthread -lboost_stacktrace_backtrace -DBOOST_STACKTRACE_USE_BACKTRACE -ldl -lssl -lcrypto
TEST_IFLAGS = -I./
TEST_BIN = test/bin/

ETHSNARK_IFLAGS = -Idepends/ethsnarks/src
ETHSNARK_SRC = depends/ethsnarks/src/jubjub/*.cpp depends/ethsnarks/src/gadgets/*.cpp  depends/ethsnarks/src/utils.cpp

# We need to use ALT_BN_128 for all ethereum related work
change-curve: check-env
	sed -i 's/DCURVE_BN128/DCURVE_ALT_BN128/g' $(PEPPER)/pepper/Makefile

make-bin: check-env
	mkdir -p $(PEPPER_BIN)
	mkdir -p $(PEPPER)/pepper/prover_verifier_shared
	mkdir -p $(TEST_BIN)
	mkdir -p scripts/bin
	mkdir -p exo_compute/bin

# Convenience target to compile gadgets 0..n
gadgets: gadget0 gadget1

gadget0: ext_gadget/sha256_bridge.cpp check-env
	$(CXX) $(CXXFLAGS) $(IFLAGS) $(ETHSNARK_IFLAGS) $< ext_gadget/common.cpp -o $(PEPPER_BIN)/gadget0 $(LDFLAGS) -lssl -lcrypto

gadget1: ext_gadget/pedersen_bridge.cpp check-env
	$(CXX) $(CXXFLAGS) $(IFLAGS) $(ETHSNARK_IFLAGS) $< ext_gadget/common.cpp $(ETHSNARK_SRC) -o $(PEPPER_BIN)/gadget1 $(LDFLAGS) -lssl -lcrypto

# Link source code of snark apps
link-apps: check-env
	ln -sf ~/dex-zksnarks/apps/* $(PEPPER)/pepper/apps/

# Link helper programs 0..n (e.g. for private input, bit decomposition, etc.)
link-exo: check-env
	ln -sf ~/dex-zksnarks/exo_compute/decompose_bits.py $(PEPPER_BIN)/exo1
	ln -sf ~/dex-zksnarks/exo_compute/private_input.py $(PEPPER_BIN)/exo0

# Target to run all tests (e.g. used by travis)
test: unit-tests e2e-tests

e2e-tests: check-env
	test/e2e/hash_transform.sh

unit-tests: $(patsubst test/apps/%.cpp, app_%, $(wildcard test/apps/*_test.cpp))

app_%_test: test/apps/%_test.cpp
	$(CXX) -g $(CXXFLAGS) -DBIGINT $(TEST_IFLAGS) $(ETHSNARK_IFLAGS) $(IFLAGS) $< -o $(TEST_BIN)/$@ $(ETHSNARK_SRC) $(TEST_LDFLAGS) $(LDFLAGS) 
	$(TEST_BIN)/$@
	
scripts: scripts/pepper_binary_format_reader.cpp make-bin
	$(CXX) $(CXXFLAGS) -DBINARY_OUTPUT $(IFLAGS) $< -o scripts/bin/pepper_binary_format_reader $(LDFLAGS)

# Make sure we have the environment set up
check-env:
ifndef PEPPER
  $(error $$PEPPER is undefined)
endif

initial: scripts make-bin change-curve all

all: link-apps link-exo gadgets