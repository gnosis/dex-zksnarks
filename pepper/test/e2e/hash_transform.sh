#!/bin/bash
set -e
source ./test/e2e/common.sh

cd $PEPPER/pepper

sed -i 's/#define ORDERS [0-9]*/#define ORDERS 2/g' apps/hash_transform.h

echo 182713746507528768497599437701574618975 > prover_verifier_shared/hash_transform.inputs
echo 032701115855630478187979287852631045544 >> prover_verifier_shared/hash_transform.inputs

runApp hash_transform

diff ./prover_verifier_shared/transform.outputs <(printf "0\n2709151994604095268253795983695773831904488672374264037853826788765176428835\n")