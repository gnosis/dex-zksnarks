#!/bin/bash
set -e

cd $PEPPER/pepper

sed -i 's/#define ORDERS [0-9]*/#define ORDERS 2/g' apps/hash_transform.h

./pepper_compile_and_setup_P.sh hash_transform
./pepper_compile_and_setup_V.sh hash_transform transform.vkey transform.pkey


echo 182713746507528768497599437701574618975 > prover_verifier_shared/transform.inputs
echo 032701115855630478187979287852631045544 >> prover_verifier_shared/transform.inputs

./bin/pepper_prover_hash_transform prove transform.pkey transform.inputs transform.outputs transform.proof
./bin/pepper_verifier_hash_transform verify transform.vkey transform.inputs transform.outputs transform.proof
