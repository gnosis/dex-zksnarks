CXX = g++

# Make sure these are the same flags that pepper was built with
CXXFLAGS = -m64 -std=c++11 -DCURVE_ALT_BN128 -DBN_SUPPORT_SNARK -UBINARY_OUTPUT -DMONTGOMERY_OUTPUT -DNO_PROCPS -DUSE_ASM

LIBSNARK = $(PEPPER)/thirdparty/libsnark
PEPPER_BIN = $(PEPPER)/pepper/bin
IFLAGS = -Iext_gadget -I$(LIBSNARK)/ -I$(LIBSNARK)/depends/libff -I$(LIBSNARK)/depends/libfqfft
LDFLAGS = -L$(LIBSNARK)/build/libsnark -L$(LIBSNARK)/build/depends/libff/libff
LDFLAGS += -lsnark -lff -lgmp -lgmpxx

# We need to use ALT_BN_128 for all ethereum related work
change-curve: check-env
	sed -i 's/DCURVE_BN128/DCURVE_ALT_BN128/g' $(PEPPER)/pepper/Makefile

# Convenience target to compile gadgets 0..n
gadgets: gadget0 gadget1

gadget0: ext_gadget/sha256_bridge.cpp check-env
	$(CXX) $(CXXFLAGS) $(IFLAGS) $< ext_gadget/gadgets/common.cpp -o $(PEPPER_BIN)/gadget0 $(LDFLAGS)

gadget1: ext_gadget/pedersen_bridge.cpp check-env
	$(CXX) $(CXXFLAGS) $(IFLAGS) $< ext_gadget/gadgets/jubjub/*.cpp -o $(PEPPER_BIN)/gadget1 $(LDFLAGS)

# Link source code of snark apps
link-apps: check-env
	ln -sf ~/pequin/apps/* $(PEPPER)/pepper/apps/

# Link helper programs 0..n (e.g. for private input, bit decomposition, etc.)
link-exo: check-env
	ln -sf ~/pequin/exo_compute/decompose_bits.py $(PEPPER_BIN)/exo1
	ln -sf ~/pequin/exo_compute/private_input.py $(PEPPER_BIN)/exo0

# Make sure we have the environment set up
check-env:
ifndef PEPPER
  $(error $$PEPPER is undefined)
endif

all: link-apps link-exo gadgets change-curve