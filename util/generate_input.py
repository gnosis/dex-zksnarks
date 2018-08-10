#!/usr/bin/python

import sys

def main():
	if len(sys.argv) > 2:
		variablesPerOrder = 1
		if (len(sys.argv) == 4):
			variablesPerOrder = int(sys.argv[3])
		orders = int(sys.argv[1])
		tokens = int(sys.argv[2])
		# orders + bitmap + volume + price vector
		argumentCount = (orders * variablesPerOrder) + (2 * orders) + (tokens - 1)
		print "time ./zokrates compute-witness -a " + " ".join(["0"] * argumentCount) + " > /dev/null"
		exit()
	printUsageAndExit()

def printUsageAndExit():
	print "\n" \
"Helper script to generate the arguments for the zokrates compute-witness command \n" + \
sys.argv[0] + " <num_orders> <num_tokens> [<order encoding in number of bits>]\n"
	exit()

if __name__ == '__main__':
   main()
