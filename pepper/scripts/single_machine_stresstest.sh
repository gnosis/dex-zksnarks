#!/bin/bash
set -e

cd $PEPPER/pepper

function run {
	DATE=$(date +%G-%M-%dT%T:000Z)
	./pepper_compile_and_setup_P.sh hash_transform | sed "s/^/[$DATE] $COUNT Setup Prover /"
	/usr/bin/time -v ./pepper_compile_and_setup_V.sh hash_transform transform.vkey transform.pkey | sed "s/^/[$DATE] $COUNT Setup Verifier /"

	/usr/bin/time -v ./bin/pepper_prover_hash_transform prove transform.pkey transform.inputs transform.outputs transform.proof | sed "s/^/[$DATE] $COUNT Prove /"
	/usr/bin/time -v ./bin/pepper_verifier_hash_transform verify transform.vkey transform.inputs transform.outputs transform.proof | sed "s/^/[$DATE] $COUNT Verify /"
}

# remove assert and provide input
sed -i 's%assert_zero(result - input->shaHash);%//assert_zero(result - input->shaHash);%g' apps/hash_transform.c
echo 62174266130574243119709474530137211798696360945565673697823515822613061023144 > prover_verifier_shared/transform.inputs

# 50M constraints
COUNT='50M'
echo '50 million constraints'
sed -i 's/#define ORDERS [0-9]*/#define ORDERS 2/g' apps/hash_transform.h
run

# 100M constraints
COUNT='100M'
date +%G-%M-%dT%T:000Z
echo '100 million constraints'
sed -i 's/#define ORDERS [0-9]*/#define ORDERS 3/g' apps/hash_transform.h
run

# 150M constraints
COUNT='150M'
date +%G-%M-%dT%T:000Z
echo '150 million constraints'
sed -i 's/#define ORDERS [0-9]*/#define ORDERS 4/g' apps/hash_transform.h
run

# 200M constraints
COUNT='200M'
date +%G-%M-%dT%T:000Z
echo '200 million constraints'
sed -i 's/#define ORDERS [0-9]*/#define ORDERS 5/g' apps/hash_transform.h
run