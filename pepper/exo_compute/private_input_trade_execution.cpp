#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <libsnark/common/default_types/r1cs_ppzksnark_pp.hpp>
#include <test/apps/declarations.h>

#include <apps/trade_execution.h>
#include <apps/parsing.h>
#include <test/apps/util.h>

using namespace std;

int main() {
    libsnark::default_r1cs_ppzksnark_pp::init_public_params();

    struct Balance balances[ACCOUNTS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {0};

    ifstream ifs("/home/docker/dex-zksnarks/exo_compute/data.json");
    Json::Reader reader;
    Json::Value obj;
    reader.parse(ifs, obj);

    // Parse prices
    auto priceMapping = obj["prices"];
    map<string, size_t> tokenIndexMapping;
    size_t tokenIndex = 0;
    for(auto tokenAndPrice = priceMapping.begin(); tokenAndPrice != priceMapping.end(); tokenAndPrice++) {
        const auto& tokenId = tokenAndPrice.key().asString();
        tokenIndexMapping[tokenId] = tokenIndex;
        prices[tokenIndex] = field254(priceMapping[tokenId.c_str()].asCString());

        // set enough balance for account 1
        balances[0].token[tokenIndex] = field254("100000000000000000000000");
        
        tokenIndex++;
    }

    // Parse orders & Volumes
    auto orderList = obj["orders"];
    size_t orderIndex = 0;
    for (const auto& order : orderList) {
        orders[orderIndex].sellToken = field254(tokenIndexMapping[order["sellToken"].asString()]);
        orders[orderIndex].buyToken = field254(tokenIndexMapping[order["buyToken"].asString()]);
        orders[orderIndex].sellAmount = field254(order["sellAmount"].asCString());
        orders[orderIndex].buyAmount = field254(order["buyAmount"].asCString());
        orders[orderIndex].account = 0;
        
        volumes[orderIndex].sellVolume = field254(order["execSellAmount"].asCString());
        volumes[orderIndex].buyVolume = field254(order["execBuyAmount"].asCString());

        //TODO surplus
        orderIndex++;
    }

    const size_t serializedLength = ORDERS*BITS_PER_ORDER + (TOKENS*BITS_PER_DECIMAL)+(3*ORDERS*BITS_PER_DECIMAL) + ACCOUNTS*TOKENS*BITS_PER_DECIMAL;
    field254* serialized = (field254*)malloc(serializedLength * sizeof(field254));
    serializeOrders(orders, serialized, 0);
    serializePricesAndVolumes(prices, volumes, serialized, ORDERS*BITS_PER_ORDER);
    serializeBalances(balances, serialized, serializedLength - (ACCOUNTS*TOKENS*BITS_PER_DECIMAL));

    for (size_t index = 0; index < serializedLength; index++) {
        cout << (serialized[index].is_zero() ? "0 " : "1 "); 
    }
    cout << endl;
    return 1;
}