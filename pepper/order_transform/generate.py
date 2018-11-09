#!/usr/bin/python3

import sys
from ethsnarks.jubjub import Point
from ethsnarks.field import FQ
from sha256.pypy_sha256 import sha256

prime = 21888242871839275222246405745257275088548364400416034343698204186575808495617

def main(numOrders):
	a = 0
	shaHash = 0
	pedersenHash = None
	for i in range(numOrders):
		order = generateRandomOrder()
		x = pedersenHash.x if pedersenHash else 0
		pedersenHash = hash_pedersen(x, order)
		shaHash = hash_sha(shaHash, order)
		
	# output sha hash
	# output pedersen hash
	print(pedersenHash)
	print(shaHash)
	print(transform_int128(shaHash))

def generateRandomOrder():
	return (2**253)-1

def hash_pedersen(left, right):
	G = Point.from_y(FQ(2626589144620713026669568689430873010625803728049924121243784502389097019475))
	H = Point.from_x(FQ(16540640123574156134436876038791482806971768689494387082833631921987005038935))
	return G * left + H * right

def transform_int128(hash):
	s = hex(hash)
	return str(int(s[:34],0)).zfill(39)+str("\n")+str(int(s[34:],16)).zfill(39)

def hash_sha(left, right):
	hash = sha256(left.to_bytes(32, 'big') + right.to_bytes(32, 'big'))
	return int(hash.hexdigest(), 16)


if __name__ == "__main__":
    main(int(sys.argv[1]))
