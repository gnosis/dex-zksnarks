#!/usr/bin/python

"""
Decomposes a given number into a 254 bit representation.
Primarily used to transfor output of a pedersen hash
(coordinates mod p) into input for the next round (bit-array)
"""

import sys
line = sys.stdin.readline()

fraction = line.split()[1] # format "numerator%denominator"

# Assuming denominator is always 1
number = int(fraction.split('%')[0])
if number < 0:
    number += 21888242871839275222246405745257275088548364400416034343698204186575808495617
assert(number < 2**254)

# Output for pepper
print " " + " ".join([i for i in "{:0254b}".format(number)])
