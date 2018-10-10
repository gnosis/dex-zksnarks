#!/bin/bash

[ -f ${BASH_SOURCE%/*}/util/assert.sh ] && . ${BASH_SOURCE%/*}/util/assert.sh

for i in ${BASH_SOURCE%/*}/*.test ; do
	echo -e "\e[2m $i \e[22m \c"
    ./zokrates compile -i $i > /dev/null
    ./zokrates compute-witness > /dev/null
    checkResponse
done

assertAllPass