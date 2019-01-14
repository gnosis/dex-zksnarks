import sys
sys.path.append('./depends/ethsnarks/')
from ethsnarks.pedersen import *

'''
This script allows creating pedersen hashes the same way the zkSnark gadget creates them.
It can be used to generate hashes of account balances & orders.
It uses the ethSnark python implementation, which is supposed to be equivalent to the
ethsnark libsnark gadget which is used by our snark.
'''

def hash(inputs):
    previous = 0
    for input in inputs:
        left = '{:0254b}'.format(previous)
        right = '{:0254b}'.format(input)
        previous = pedersen_hash_zcash_bits("dex.pedersen-hash", left + right).x.n
    return previous

def main():
    if len(sys.argv) == 1:
        print("Usage: %s [space separated list of elements to hash]" % sys.argv[0])
        exit(1)

    numbers = list(map(lambda input: int(input), sys.argv[1:]))
    print(hash(numbers))

if __name__== "__main__":
  main()