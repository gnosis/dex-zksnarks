#include <cassert>
#include <algorithm>
#include <gtest/gtest.h>
#include "declarations.h"

#define EXT_GADGET_OVERRIDE
struct Private {};

#include <apps/hashing.h>
#include "util.h"

// Record of all bit vectors passed into the pedersen gadget
std::vector< std::vector<field254> > pedersenCalls;
// Record of all bit vectors passed into the sha gadget
std::vector< std::vector<field254> > shaCalls;

// Mocked result for PedersenGadget
struct PedersenHash pedersenHashResult;
// Mocked result for shaGadget
struct ShaHash shaResult;

// Fake implementation of ext_gadget populating values above
void ext_gadget(void* in, void* out, uint32_t gadget) {
    field254* boolIn = (field254*) in;
    switch(gadget) {
        case 0: {
            std::vector<field254> b(boolIn, boolIn + SHA_HASH_SIZE);
            shaCalls.push_back(b);
            struct ShaHash* shaOut = (ShaHash*)out;
            copyBits(shaResult.digest, 0, shaOut->digest, 0, SHA_HASH_SIZE);
            break;
        }
        case 1: {
            std::vector<field254> b(boolIn, boolIn + PEDERSEN_HASH_SIZE);
            pedersenCalls.push_back(b);
            struct PedersenHash* pedersenOut = (PedersenHash*)out;
            pedersenOut->values[0] = pedersenHashResult.values[0];
            pedersenOut->values[1] = pedersenHashResult.values[1];
            break;
        }
        default:
            assert(false);
            break;
    }
}

#pragma mark - Pedersen

TEST(HashPedersenTest, SingleChunk) { 
    pedersenCalls.clear();
    field254 input[6] = {0, 0, 0, 1, 1, 1};
    std::vector<field254> inputV(input, input+6);

    hashPedersen(input, 6, 6);

    ASSERT_EQ(pedersenCalls.size(), 1);
    auto& first = pedersenCalls.front();
    ASSERT_TRUE(std::equal(first.cbegin() + PEDERSEN_HASH_SIZE - 6, first.cend(), inputV.cbegin()));
}

TEST(HashPedersenTest, MultipleChunks) {
    pedersenCalls.clear();
    field254 input[6] = {0, 0, 0, 1, 1, 1};
    std::vector<field254> inputV(input, input+6);
    
    hashPedersen(input, 6, 2);
    
    ASSERT_EQ(pedersenCalls.size(), 3);
    auto& el = pedersenCalls[0];
    ASSERT_TRUE(std::equal(el.cbegin() + PEDERSEN_HASH_SIZE - 2, el.cend(), inputV.cbegin()));

    el = pedersenCalls[1];
    ASSERT_TRUE(std::equal(el.cbegin() + PEDERSEN_HASH_SIZE - 2 , el.cend(), inputV.cbegin() + 2));

    el = pedersenCalls[2];
    ASSERT_TRUE(std::equal(el.cbegin() + PEDERSEN_HASH_SIZE - 2, el.cend(), inputV.cbegin() + 4));
}

TEST(HashPedersenTest, ReusesXCoordinateInNextRound) {
    pedersenCalls.clear();
    pedersenHashResult = { {1, 0} };
    field254 input[6] = {0};
    
    auto result = hashPedersen(input, 6, 3);
    ASSERT_EQ(pedersenCalls.size(), 2);

    // Result is X Coordinate
    ASSERT_EQ(result, 1);

    // Starting point is 0
    auto& el = pedersenCalls[0];
    ASSERT_TRUE(std::all_of(el.cbegin(), el.cbegin() + 254, [](field254 i){return i==0;}));

    // Next iteration uses previous x coorindate
    el = pedersenCalls[1];
    ASSERT_EQ(el[253], 1);
}

#pragma mark - SHA

TEST(HashSHATest, SingleChunk) { 
    shaCalls.clear();
    field254 input[6] = {0, 0, 0, 1, 1, 1};
    std::vector<field254> inputV(input, input+6);

    hashSHA(input, 6, 6);

    ASSERT_EQ(shaCalls.size(), 1);
    auto& first = shaCalls.front();
    ASSERT_TRUE(std::equal(first.cbegin() + SHA_HASH_SIZE - 6 , first.cend(), inputV.cbegin()));
}

TEST(HashSHATest, MultipleChunks) { 
    shaCalls.clear();
    field254 input[6] = {0, 0, 0, 1, 1, 1};
    std::vector<field254> inputV(input, input+6);

    hashSHA(input, 6, 2);

    ASSERT_EQ(shaCalls.size(), 3);
    auto& el = shaCalls[0];
    ASSERT_TRUE(std::equal(el.cbegin() + SHA_HASH_SIZE - 2, el.cend(), inputV.cbegin()));

    el = shaCalls[1];
    ASSERT_TRUE(std::equal(el.cbegin() + SHA_HASH_SIZE - 2, el.cend(), inputV.cbegin() + 2));

    el = shaCalls[2];
    ASSERT_TRUE(std::equal(el.cbegin() + SHA_HASH_SIZE - 2, el.cend(), inputV.cbegin() + 4));
}

TEST(HashSHATest, ReuseResultInNextRound) { 
    shaCalls.clear();
    shaResult = { { 0 } };
    shaResult.digest[255] = 1;

    field254 input[6] = { 0 };
    hashSHA(input, 6, 3);

    ASSERT_EQ(pedersenCalls.size(), 2);
    
    // Starting point is 0
    auto& el = shaCalls[0];
    ASSERT_TRUE(std::all_of(el.cbegin(), el.cbegin() + 256, [](field254 i){return i==0;}));

    // Next iteration uses previous digest
    el = shaCalls[1];
    ASSERT_EQ(el[255], 1);
}

TEST(HashSHATest, SplitsResult) { 
    shaCalls.clear();
    shaResult = { { 0 } };
    // shaResult will be [0, ..0, 1][0, ..0, 1, 1] => [1, 3]
    shaResult.digest[127] = 1;
    shaResult.digest[254] = 1;
    shaResult.digest[255] = 1;

    field254 input[6] = { 0 };
    ShaResult result = hashSHA(input, 6, 6);

    ASSERT_EQ(result.left, 1);
    ASSERT_EQ(result.right, 3);
}

int main(int argc, char **argv) {
    testing::FLAGS_gtest_death_test_style = "threadsafe";
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}