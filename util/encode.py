#!/usr/bin/python

import sys

def encodeNumber(number):
	return int(number * 10**18)

def encodeOrder(amount, sourceToken, targetToken, limit):
	amount = encodeNumber(amount)
	limit = encodeNumber(limit)
	return int(amount + (sourceToken << 120) + (targetToken << 125) + (limit << 130))

def main():
	if len(sys.argv) > 1:
		action = sys.argv[1]
		if action == 'number' and len(sys.argv) == 3:
		  	print encodeNumber(float(sys.argv[2]))
		  	return;
		elif action == 'order' and len(sys.argv) == 6:
			print encodeOrder(float(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]), float(sys.argv[5]))
			return
	printUsageAndExit()

def printUsageAndExit():
	print "\n" \
"Helper script to encode information into the format the snark proof assumes \n\n\
Usage: \n" + \
sys.argv[0] + " number <number>\n" + \
sys.argv[0] + " order <amount> <source token id> <target token id> <limit>\n"
	exit()

if __name__ == '__main__':
   main()