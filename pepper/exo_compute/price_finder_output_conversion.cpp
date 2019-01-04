#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <libsnark/common/default_types/r1cs_ppzksnark_pp.hpp>
#include <test/apps/declarations.h>

#include <apps/trade_execution.h>
#include <apps/parsing.h>
#include <test/apps/util.h>

using namespace std;

string fieldToString(field254 field) {
    mpz_t t; mpz_init(t);
    field.as_bigint().to_mpz(t);
    return mpz_get_str(NULL,10,t);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        cout << endl << "Script, that allows to convert the solver json output into public and private input (exo_compute) files, required by pepper." << endl << endl;
        cout << "Usage:" << endl << "  " << argv[0] << " <data file> <public input target> <private input target>" << endl;
        return -1;
    }
    libsnark::default_r1cs_ppzksnark_pp::init_public_params();

    field254 balances[ACCOUNTS*TOKENS] = {0};
    struct Order orders[ORDERS] = {0};
    struct Volume volumes[ORDERS] = {0};
    field254 prices[TOKENS] = {0};

    ifstream ifs(argv[1]);
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
        tokenIndex++;
    }

    auto accounts = obj["accounts"];
    map<string, size_t> accountIndexMapping;
    size_t accountIndex = 0;
    for (auto accountAndBalances = accounts.begin(); accountAndBalances != accounts.end(); accountAndBalances++) {
        const auto& accountId = accountAndBalances.key().asString();
        accountIndexMapping[accountId] = accountIndex;
        auto& tokens = accounts[accountId.c_str()];
        for (auto tokenAndBalance = tokens.begin(); tokenAndBalance != tokens.end(); tokenAndBalance++) {
            const auto& tokenId = tokenAndBalance.key().asString();
            auto index = (accountIndex * TOKENS) + tokenIndexMapping[tokenId];
            balances[index] = field254(tokens[tokenId.c_str()].asCString());
        }
        accountIndex++;
    }

    const size_t serializedLength = ORDERS*BITS_PER_ORDER + (TOKENS*BITS_PER_DECIMAL)+(4*ORDERS*BITS_PER_DECIMAL) + ACCOUNTS*TOKENS*BITS_PER_DECIMAL;
    field254* serialized = (field254*)malloc(serializedLength * sizeof(field254));
    serializeBalances(balances, serialized, serializedLength - (ACCOUNTS*TOKENS*BITS_PER_DECIMAL));

    // Parse orders & Volumes
    auto orderList = obj["orders"];
    size_t orderIndex = 0;
    field254 totalSurplus = 0;
    for (const auto& order : orderList) {
        orders[orderIndex].sellToken = field254(tokenIndexMapping[order["sellToken"].asString()]);
        orders[orderIndex].buyToken = field254(tokenIndexMapping[order["buyToken"].asString()]);
        orders[orderIndex].sellAmount = field254(order["sellAmount"].asCString());
        orders[orderIndex].buyAmount = field254(order["buyAmount"].asCString());
        orders[orderIndex].account = field254(accountIndexMapping[order["accountID"].asString()]);
        
        volumes[orderIndex].sellVolume = field254(order["execSellAmount"].asCString());
        volumes[orderIndex].buyVolume = field254(order["execBuyAmount"].asCString());
        volumes[orderIndex].surplus = field254(order["execSurplus"].asCString());

        //adjust balance
        auto sellIndex = (orders[orderIndex].account.as_ulong() * TOKENS) + orders[orderIndex].sellToken.as_ulong();
        auto buyIndex = (orders[orderIndex].account.as_ulong() * TOKENS) + orders[orderIndex].buyToken.as_ulong();
        balances[sellIndex] -= volumes[orderIndex].sellVolume;
        balances[buyIndex] += volumes[orderIndex].buyVolume;

        totalSurplus += volumes[orderIndex].surplus;
        orderIndex++;
    }

    serializeOrders(orders, serialized, 0);
    serializePricesAndVolumes(prices, volumes, serialized, ORDERS*BITS_PER_ORDER);

    ofstream privateInputFile(argv[3]);
    privateInputFile << "#!/bin/bash" << endl << "echo \"";
    for (size_t index = 0; index < serializedLength; index++) {
        privateInputFile << (serialized[index].is_zero() ? "0 " : "1 "); 
    }
    privateInputFile << "\"";
    privateInputFile.close();

    // Export Public input
    field254 orderHash = hashPedersen(serialized, 0, ORDERS*BITS_PER_ORDER, BITS_PER_ORDER);
    ShaResult hashBatchInfo = hashSHA(serialized, ORDERS*BITS_PER_ORDER, (TOKENS*BITS_PER_DECIMAL)+(ORDERS*BITS_PER_DECIMAL), 256);
    field254 balanceHash = hashPedersen(serialized, serializedLength - (ACCOUNTS*TOKENS*BITS_PER_DECIMAL), ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL);

    ofstream publicInputFile(argv[2]);
    publicInputFile << fieldToString(balanceHash) << endl;
    publicInputFile << fieldToString(totalSurplus) << endl;
    publicInputFile << fieldToString(hashBatchInfo.left) << endl;
    publicInputFile << fieldToString(hashBatchInfo.right) << endl;
    publicInputFile << fieldToString(orderHash) << endl;
    publicInputFile << "10000000000000"; //epsilon
    publicInputFile.close();

    // Print expected output
    cout << "0" <<endl;
    field254* updatedSerializedBalances = (field254*)malloc(ACCOUNTS*TOKENS*BITS_PER_DECIMAL * sizeof(field254));
    serializeBalances(balances, updatedSerializedBalances, 0);
    hashPedersen(updatedSerializedBalances, 0, ACCOUNTS*TOKENS*BITS_PER_DECIMAL, BITS_PER_DECIMAL).print();

    return 0;
}