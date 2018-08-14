#!/bin/bash

PASS='\033[0;32m[\xE2\x9C\x94]\033[0m'
FAIL='\033[0;31m[\xE2\x9D\x8C]\033[0m'

set -e

for i in ${BASH_SOURCE%/*}/*.test ; do
	echo -e "\e[2m $i \e[22m \c"
    ./zokrates compile -i $i > /dev/null
    ./zokrates compute-witness > /dev/null
    if [ $? -eq 0 ]; then
    	echo -e "${PASS}"
    else
    	echo -e "${FAIL}"
    fi
done