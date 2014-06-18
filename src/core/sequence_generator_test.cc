#include "core/sequence_generator.h"

#include <string>
#include <map>

#include <gtest/gtest.h>

using namespace std;

TEST(SequenceGeneratorTest, CheckRestriction)
{
    KumipuyoSeq seq = generateSequence();

    EXPECT_EQ(128, seq.size());

    // The first 3 hand should be RED, BLUE or YELLOW.
    for (int i = 0; i < 3; ++i) {
        EXPECT_NE(PuyoColor::GREEN, seq.axis(i));
        EXPECT_NE(PuyoColor::GREEN, seq.child(i));
    }

    map<PuyoColor, int> count;
    for (int i = 0; i < seq.size(); ++i) {
        count[seq.axis(i)]++;
        count[seq.child(i)]++;
    }

    EXPECT_EQ(4, count.size());
    EXPECT_EQ(64, count[PuyoColor::RED]);
    EXPECT_EQ(64, count[PuyoColor::BLUE]);
    EXPECT_EQ(64, count[PuyoColor::YELLOW]);
    EXPECT_EQ(64, count[PuyoColor::GREEN]);
}