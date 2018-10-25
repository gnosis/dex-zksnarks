#!/bin/bash
set -e

cd $PEPPER/pepper

sed -i 's/#define ORDERS [0-9]*/#define ORDERS 2/g' apps/hash_transform.c

./pepper_compile_and_setup_P.sh hash_transform
./pepper_compile_and_setup_V.sh hash_transform transform.vkey transform.pkey

echo 62174266130574243119709474530137211798696360945565673697823515822613061023144 > prover_verifier_shared/transform.inputs

./bin/pepper_prover_hash_transform prove transform.pkey transform.inputs transform.outputs transform.proof
./bin/pepper_verifier_hash_transform verify transform.vkey transform.inputs transform.outputs transform.proof
