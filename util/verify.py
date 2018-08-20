#!/usr/bin/python

import sys
import json
from helper.json_parsing import parseJsonData
from encode import encodeNumber

def main():
  if len(sys.argv) == 2:
    with open(sys.argv[1]) as f:
        data = json.load(f)
        orders, bitmap, volumes, prices = parseJsonData(data, len(data["sellOrders"]), len(data["tokens"]))
        verify(orders, bitmap, volumes, prices)
  else:
    printUsageAndExit()

def verify(orders, bitmap, volumes, prices):
    failed = False
    priceMap = dict(zip(range(1, len(prices)+1), prices))
    priceMap[0] = encodeNumber(1)

    buyVolumes = dict(zip(range(0, len(prices)+1), [0] * (len(prices)+1)))
    sellVolumes = dict(zip(range(0, len(prices)+1), [0] * (len(prices)+1)))
    highestTouchedLimits = {}
    for i in range(len(orders)):
        order = orders[i]
        volume = volumes[i]
        if volume > 0:
            # Check we respected the limit price
            lhs = priceMap[order.sourceToken] * encodeNumber(1)
            rhs = priceMap[order.targetToken] * encodeNumber(order.limit)
            if lhs < rhs:
                failed = True
                difference = (lhs-rhs)*10**-36
                print 'Limit price mismatch for order {}({}): {} should be >= {}, Difference: {} ETH'.format(i+1, order.id, lhs, rhs, difference)

            buyVolumes[order.targetToken] += volume * priceMap[order.sourceToken]
            sellVolumes[order.sourceToken] += volume * priceMap[order.sourceToken]

            if highestTouchedLimits.get(tokenPairKey(order), 0) < order.limit:
                highestTouchedLimits[tokenPairKey(order)] = order.limit

    # Check buy/sell volumes match
    for key in buyVolumes:
        if buyVolumes[key] != sellVolumes[key]:
            failed= True
            difference = (buyVolumes[key]-sellVolumes[key])*10**-36
            print 'Token {} Buy volume: {} != Sell Volume: {}, Difference: {} ETH'.format(key, buyVolumes[key], sellVolumes[key], difference)

    #Check fairness
    for i in range(len(orders)):
        order = orders[i]
        highestTouchedLimit = highestTouchedLimits.get(tokenPairKey(order), 0)
        if  highestTouchedLimit > order.limit and (encodeNumber(order.amount) - volumes[i]) != 0:
            failed = True
            print 'Order {}({}) with limit {} only partially fulfilled although an order with limit {} was touched'.format(i+1, order.id, order.limit, highestTouchedLimit)

    if failed:
        exit(1)

def tokenPairKey(order):
    return str(order.sourceToken)+str(order.targetToken)

def printUsageAndExit():
    print "\n" \
"Helper script to quickly verify if the output of the solver is correct \n" + \
sys.argv[0] + " [<json result file>]\n"
    exit()

if __name__ == '__main__':
    main()