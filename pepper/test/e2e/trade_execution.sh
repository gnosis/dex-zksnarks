#!/bin/bash
set -e
source ./test/e2e/common.sh

sed -i 's/#define ORDERS [0-9]*/#define ORDERS 3/g' apps/trade_execution.h
sed -i 's/#define TOKENS [0-9]*/#define TOKENS 3/g' apps/trade_execution.h
sed -i 's/#define ACCOUNTS [0-9]*/#define ACCOUNTS 1/g' apps/trade_execution.h
make price_finder_output_conversion
make link-apps

echo "Converting json to private/public input:"
./exo_compute/bin/price_finder_output_conversion ./test/e2e/trade_execution.json $PEPPER/pepper/prover_verifier_shared/trade_execution.inputs $PEPPER/pepper/bin/exo2
chmod +x $PEPPER/pepper/bin/exo2

runApp trade_execution

diff $PEPPER/pepper/prover_verifier_shared/trade_execution.outputs <(printf "0\n6490902738647173036319167296619824463594166904014951787745877661502068588144\n")