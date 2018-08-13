#!/usr/bin/python

import sys
import json
from encode import encodeOrder, encodeNumber

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
        encodedOrders, bitmap, volumes, prices = parseJsonData(data, orders, tokens)

        writeArguments(encodedOrders, bitmap, volumes, prices, variablesPerOrder)
        
        print "time ./zokrates compute-witness --interactive < data.input > /dev/null"
        exit()
    printUsageAndExit()

def parseJsonData(data, maxOrders, maxTokens):
    if not data:
        return [0] * maxOrders, [0] * maxOrders, [0] * maxOrders, [0] * (maxTokens-1)

    refToken = data["refToken"]
    refTokenPrice = data["pricesNew"][refToken]

    tokenList = data["tokens"]
    tokenList.remove
    assert(len(tokenList) <= maxTokens);
    tokenMapping = dict(zip(tokenList, range(len(tokenList))))

    priceList = [0] * maxTokens
    for currency, newPrice in data["pricesNew"].iteritems():
        if currency == refToken:
            continue
        index = tokenMapping[currency]
        priceList[index] = encodeNumber(refTokenPrice/newPrice)
    # We want to ignore the ref token in the price vector
    del priceList[tokenMapping[refToken]]

    encodedOrders = [0] * maxOrders
    bitmap = [0] * maxOrders
    volume = [0] * maxOrders

    jsonOrders = data["sellOrders"]
    for i in range(len(jsonOrders)):
        order = jsonOrders[i]

        amount = order["sellAmount"]
        sourceToken = tokenMapping[order["sellToken"]]
        targetToken = tokenMapping[order["buyToken"]]
        limit = order["limitRate"][1][0]

        encodedOrders[i] = encodeOrder(amount, sourceToken, targetToken, limit)
        bitmap[i] = 1
        volume[i] = encodeNumber(order["execSellAmount"])
    return encodedOrders, bitmap, volume, priceList

def writeArguments(encodedOrders, bitmap, volumes, prices, variablesPerOrder):
    with open('data.input', 'w') as f:
        for order in encodedOrders:
            if variablesPerOrder > 1:
                f.write('\n'.join(reversed('{0:0253b}'.format(order))))
                f.write('\n')
            else:
                f.write(str(order) + "\n")

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