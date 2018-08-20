#!/bin/bash

[ -f ${BASH_SOURCE%/*}/util/assert.sh ] && . ${BASH_SOURCE%/*}/util/assert.sh

echo "Bitwise Encodig"

echo -e "  \e[2mGenerating Source\e[22m \c"
node ${BASH_SOURCE%/*}/../codegen/generate.js --template ${BASH_SOURCE%/*}/../codegen/templates/verify_constraints_bitwise_order_encoding.ejs --out ${BASH_SOURCE%/*}/../verify3orders3tokens_bitwise.code -o 3 -t 3 > /dev/null
checkResponse

echo -e "  \e[2mComparing Source\e[22m \c"
diff ${BASH_SOURCE%/*}/../verify3orders3tokens_bitwise.code ${BASH_SOURCE%/*}/../ringtrade_example_bitwise.code
checkResponse

echo -e "  \e[2mCompiling Source\e[22m \c"
./zokrates compile -i ${BASH_SOURCE%/*}/../ringtrade_example_bitwise.code > /dev/null
checkResponse

echo -e "  \e[2mRunning with 0 example\e[22m \c"
${BASH_SOURCE%/*}/../util/generate_input.py 3 3 253 | source /dev/stdin > /dev/null 2>&1
checkResponse

echo -e "  \e[2mRunning with json example\e[22m \c"
${BASH_SOURCE%/*}/../util/generate_input.py 3 3 253 ${BASH_SOURCE%/*}/../examples/ringtrade.json | source /dev/stdin > /dev/null 2>&1
checkResponse

echo "Non-Bitwise Encodig"

echo -e "  \e[2mGenerating Source\e[22m \c"
node ${BASH_SOURCE%/*}/../codegen/generate.js --template ${BASH_SOURCE%/*}/../codegen/templates/verify_constraints.ejs --out ${BASH_SOURCE%/*}/../verify3orders3tokens.code -o 3 -t 3 > /dev/null
checkResponse

echo -e "  \e[2mComparing Source\e[22m \c"
diff ${BASH_SOURCE%/*}/../verify3orders3tokens.code ${BASH_SOURCE%/*}/../ringtrade_example.code
checkResponse

echo -e "  \e[2mCompiling Source\e[22m \c"
./zokrates compile -i ${BASH_SOURCE%/*}/../ringtrade_example.code > /dev/null
checkResponse

echo -e "  \e[2mRunning with 0 example\e[22m \c"
${BASH_SOURCE%/*}/../util/generate_input.py 3 3 1 | source /dev/stdin > /dev/null 2>&1
checkResponse

echo -e "  \e[2mRunning with json example\e[22m \c"
${BASH_SOURCE%/*}/../util/generate_input.py 3 3 1 ${BASH_SOURCE%/*}/../examples/ringtrade.json | source /dev/stdin > /dev/null 2>&1
checkResponse

assertAllPass