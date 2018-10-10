#!/usr/bin/python

import sys
import json
from helper.json_parsing import parseJsonData

def main():
    if len(sys.argv) > 2:
        orders = int(sys.argv[1])
        tokens = int(sys.argv[2])

        variablesPerOrder = 1
        if (len(sys.argv) >= 4):
            variablesPerOrder = int(sys.argv[3])

        data = {}
        if (len(sys.argv) == 5):
            with open(sys.argv[4]) as f:
                data = json.load(f)
        orders, bitmap, volumes, prices = parseJsonData(data, orders, tokens)

        writeArguments(orders, bitmap, volumes, prices, variablesPerOrder)
        
        print "/usr/bin/time -v ./zokrates compute-witness --interactive < data.input > /dev/null"
        exit()
    printUsageAndExit()

def writeArguments(orders, bitmap, volumes, prices, variablesPerOrder):
    with open('data.input', 'w') as f:
        for order in orders:
            encodedOrder = order.encode()
            if variablesPerOrder > 1:
                f.write('\n'.join(reversed('{0:0253b}'.format(encodedOrder))))
                f.write('\n')
            else:
                f.write(str(encodedOrder) + "\n")

        for bit in bitmap:
            f.write(str(bit) + "\n")

        for volume in volumes:
            f.write(str(volume) + "\n")

        for price in prices:
            f.write(str(price) + "\n")

def printUsageAndExit():
    print "\n" \
"Helper script to generate the arguments for the zokrates compute-witness command \n" + \
sys.argv[0] + " <num_orders> <num_tokens> [<order encoding in number of bits>] [<json result file>]\n"
    exit()

if __name__ == '__main__':
    main()