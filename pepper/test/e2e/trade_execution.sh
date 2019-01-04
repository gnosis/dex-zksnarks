#!/bin/bash
set -e
source ./test/e2e/common.sh

function runTradeExecution() {
    make price_finder_output_conversion
    make link-apps

    echo "Converting json to private/public input:"
    ./exo_compute/bin/price_finder_output_conversion $1 $PEPPER/pepper/prover_verifier_shared/trade_execution.inputs $PEPPER/pepper/bin/exo2 > ./test/e2e/trade_execution.expected_output
    chmod +x $PEPPER/pepper/bin/exo2

    runApp trade_execution

    diff $PEPPER/pepper/prover_verifier_shared/trade_execution.outputs ./test/e2e/trade_execution.expected_output
}

# Small example
sed -i 's/#define ORDERS [0-9]*/#define ORDERS 3/g' apps/trade_execution.h
sed -i 's/#define TOKENS [0-9]*/#define TOKENS 3/g' apps/trade_execution.h
sed -i 's/#define ACCOUNTS [0-9]*/#define ACCOUNTS 1/g' apps/trade_execution.h
runTradeExecution ./test/e2e/trade_execution.json

# Larger example
sed -i 's/#define ORDERS [0-9]*/#define ORDERS 10/g' apps/trade_execution.h
sed -i 's/#define TOKENS [0-9]*/#define TOKENS 3/g' apps/trade_execution.h
sed -i 's/#define ACCOUNTS [0-9]*/#define ACCOUNTS 3/g' apps/trade_execution.h
runTradeExecution ./test/e2e/larger_trade_execution.json