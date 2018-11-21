#include <cassert>
#include <gtest/gtest.h>
#include "declarations.h"

#define EXT_GADGET_OVERRIDE
struct Private {};

#include <apps/hashing.h>
#include "util.h"

std::vector< std::vector<bool> > pedersenCalls;
void ext_gadget(void* in, void* out, uint32_t gadget) {
    assert(gadget == 1); // PedersenHashGadget
    bool *boolIn = (bool*) in;
    std::vector<bool> b(boolIn, boolIn + 508);
    pedersenCalls.push_back(b);
}

TEST(HashPedersenTest, SingleChunk) { 
    pedersenCalls.clear();
    bool input[6] = {0, 0, 0, 1, 1, 1};
    std::vector<bool> inputV(input, input+6);

    hashPedersen(input, 6, 6);

    ASSERT_EQ(pedersenCalls.size(), 1);
    auto& first = pedersenCalls.front();
    ASSERT_EQ(first.size(), 508);
    ASSERT_TRUE(std::equal(first.cbegin() + 502, first.cend(), inputV.cbegin()));
}

TEST(HashPedersenTest, MultipleChunks) {
    pedersenCalls.clear();
    bool input[6] = {0, 0, 0, 1, 1, 1};
    std::vector<bool> inputV(input, input+6);
    
    hashPedersen(input, 6, 2);
    
    ASSERT_EQ(pedersenCalls.size(), 3);
    auto& el = pedersenCalls[0];
    ASSERT_EQ(el.size(), 508);
    ASSERT_TRUE(std::equal(el.cbegin() + 506, el.cend(), inputV.cbegin()));

    el = pedersenCalls[1];
    ASSERT_EQ(el.size(), 508);
    ASSERT_TRUE(std::equal(el.cbegin() + 506, el.cend(), inputV.cbegin() + 2));

    el = pedersenCalls[2];
    ASSERT_EQ(el.size(), 508);
    ASSERT_TRUE(std::equal(el.cbegin() + 506, el.cend(), inputV.cbegin() + 4));
}

int main(int argc, char **argv) {
    testing::FLAGS_gtest_death_test_style = "threadsafe";
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}