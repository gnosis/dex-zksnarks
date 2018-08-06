#!/bin/bash

NC='\033[0m' # No Color
PASS='\033[0;32m[\xE2\x9C\x94]'
FAIL='\033[0;31m[\xE2\x9D\x8C]'

echo "Compiling Source"
./zokrates compile -i ${BASH_SOURCE%/*}/../ringtrade_example.code > /dev/null
echo "Compilation complete"

# Orders:
# 10, ETH, GNO, 5
# 10, GNO, ETH, .1
# 10, GNO, ETH  .2
# 
# Bitmap 1 1 1
# Volumes 2 0 10
# Prices 5 0
TESTNAME='Better order withholdal'
./zokrates compute-witness -a 6805647338418769269310027444500481537062921825928971026432 136112946768375385386679070968492200465303807060280344576 272225893536750770772028913941199485047703807060280344576 1 1 1 2000000000000000000 0 10000000000000000000 5000000000000000000 0 > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${FAIL} ${TESTNAME}. Order2 should be fulfilled before order3${NC}"
else
    echo -e "${PASS} ${TESTNAME}${NC}"
fi

# Orders:
# 10, ETH, GNO, 5
# 10, GNO, ETH, .1
# 0
# 
# Bitmap 1 1 0
# Volumes 2 10 0
# Prices 10 0
TESTNAME='Buy/Selll volume mismatch'
./zokrates compute-witness -a 6805647338418769269310027444500481537062921825928971026432 136112946768375385386679070968492200465303807060280344576 0 1 1 0 2000000000000000000 10000000000000000000 0 10000000000000000000 0 > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${FAIL} ${TESTNAME}. We sell more ETH than we buy${NC}"
else
    echo -e "${PASS} ${TESTNAME}${NC}"
fi

# Orders:
# 10, ETH, GNO, 5
# 10, GNO, ETH, .1
# 0
# 
# Bitmap 1 1 0
# Volumes 10 10 0
# Prices 1 0
TESTNAME='Price above limit'
./zokrates compute-witness -a 6805647338418769269310027444500481537062921825928971026432 136112946768375385386679070968492200465303807060280344576 0 1 1 0 10000000000000000000 10000000000000000000 0 1000000000000000000 0 > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${FAIL} ${TESTNAME}. Orders should only be touched, if the limit price is below the calculated price${NC}"
else
    echo -e "${PASS} ${TESTNAME}${NC}"
fi

# Orders:
# 10, ETH, GNO, 5
# 5, GNO, ETH, .1
# 0
# 
# Bitmap 1 1 0
# Volumes 1 10 0
# Prices 10 0
TESTNAME='Volume above order offer'
./zokrates compute-witness -a 6805647338418769269310027444500481537062921825928971026432 136112946768375385386679070968492200460303807060280344576 0 1 1 0 1000000000000000000 10000000000000000000 0 10000000000000000000 0 > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${FAIL} ${TESTNAME}. Volume should not be allowed larger than specified in the offer${NC}"
else
    echo -e "${PASS} ${TESTNAME}${NC}"
fi