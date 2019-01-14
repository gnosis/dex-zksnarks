#!/usr/bin/python3

import sys
from sha256.pypy_sha256 import sha256
sys.path.append('./scripts/')
import dex_hash_pedersen

def main(numOrders):
	a = 0
	shaHash = 0
	orders = []
	for i in range(numOrders):
		order = generateRandomOrder()
		orders.append(order)
		shaHash = hash_sha(shaHash, order)
	pedersenHash = dex_hash_pedersen.hash(orders)
		
	# output pedersen hash
	# output sha hash transformed to two int128
	print(pedersenHash)
	print(transform_int128(shaHash))

def generateRandomOrder():
	return (2**253)-1

#transform_int128 transform a 256 bit value into two 128 bit values by cutting the hash 
# into two substrings and representing these substrings as ints with 39 digits 
def transform_int128(hash):
	s = hex(hash)
	return [str(int(s[:34],0)).zfill(39),str(int(s[34:],16)).zfill(39)]

def hash_sha(left, right):
	hash = sha256(left.to_bytes(32, 'big') + right.to_bytes(32, 'big'))
	return int(hash.hexdigest(), 16)


if __name__ == "__main__":
	main(int(sys.argv[1]))
