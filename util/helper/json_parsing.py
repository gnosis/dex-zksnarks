import sys
sys.path.append("..")
from encode import encodeOrder, encodeNumber

class Order:
    def __init__(self, amount, sourceToken, targetToken, limit):
        self.amount = amount
        self.sourceToken = sourceToken
        self.targetToken = targetToken
        self.limit = limit

    def encode(self):
        return encodeOrder(self.amount, self.sourceToken, self.targetToken, self.limit);

def parseJsonData(data, maxOrders, maxTokens):
    if not data:
        return [Order(0, 0, 0, 0)] * maxOrders, [0] * maxOrders, [0] * maxOrders, [0] * (maxTokens-1)

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
        priceList[index] = encodeNumber(newPrice/refTokenPrice)
    # We want to ignore the ref token in the price vector
    del priceList[tokenMapping[refToken]]

    encodedOrders = [Order(0, 0, 0, 0)] * maxOrders
    bitmap = [0] * maxOrders
    volume = [0] * maxOrders

    jsonOrders = data["sellOrders"]
    for i in range(len(jsonOrders)):
        order = jsonOrders[i]

        amount = order["sellAmount"]
        sourceToken = tokenMapping[order["sellToken"]]
        targetToken = tokenMapping[order["buyToken"]]
        limit = order["limitRate"][1][0]

        encodedOrders[i] = Order(amount, sourceToken, targetToken, limit)
        bitmap[i] = 1
        volume[i] = encodeNumber(min(amount, order["execSellAmount"]))
    return encodedOrders, bitmap, volume, priceList