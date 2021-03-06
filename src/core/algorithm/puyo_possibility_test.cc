#include "puyo_possibility.h"

#include <gtest/gtest.h>

TEST(TsumoPossibilityTest, possibility)
{
    TsumoPossibility::initialize();

    EXPECT_EQ(1.0, TsumoPossibility::possibility(PuyoSet(0, 0, 0, 0), 0));

    EXPECT_EQ(4.0 / 4.0, TsumoPossibility::possibility(PuyoSet(0, 0, 0, 0), 1));
    EXPECT_EQ(1.0 / 4.0, TsumoPossibility::possibility(PuyoSet(1, 0, 0, 0), 1));
    EXPECT_EQ(1.0 / 4.0, TsumoPossibility::possibility(PuyoSet(0, 1, 0, 0), 1));
    EXPECT_EQ(1.0 / 4.0, TsumoPossibility::possibility(PuyoSet(0, 0, 1, 0), 1));
    EXPECT_EQ(1.0 / 4.0, TsumoPossibility::possibility(PuyoSet(0, 0, 0, 1), 1));
    EXPECT_EQ(0.0 / 4.0, TsumoPossibility::possibility(PuyoSet(1, 1, 0, 0), 1));
    EXPECT_EQ(0.0 / 4.0, TsumoPossibility::possibility(PuyoSet(0, 1, 1, 0), 1));
    EXPECT_EQ(0.0 / 4.0, TsumoPossibility::possibility(PuyoSet(0, 0, 1, 1), 1));
    EXPECT_EQ(0.0 / 4.0, TsumoPossibility::possibility(PuyoSet(1, 0, 0, 1), 1));

    EXPECT_EQ(16.0 / 16.0, TsumoPossibility::possibility(PuyoSet(0, 0, 0, 0), 2));

    EXPECT_EQ(7.0 / 16.0, TsumoPossibility::possibility(PuyoSet(1, 0, 0, 0), 2));
    EXPECT_EQ(7.0 / 16.0, TsumoPossibility::possibility(PuyoSet(0, 1, 0, 0), 2));
    EXPECT_EQ(7.0 / 16.0, TsumoPossibility::possibility(PuyoSet(0, 0, 1, 0), 2));
    EXPECT_EQ(7.0 / 16.0, TsumoPossibility::possibility(PuyoSet(0, 0, 0, 1), 2));

    EXPECT_EQ(1.0 / 16.0, TsumoPossibility::possibility(PuyoSet(2, 0, 0, 0), 2));
    EXPECT_EQ(1.0 / 16.0, TsumoPossibility::possibility(PuyoSet(0, 2, 0, 0), 2));
    EXPECT_EQ(1.0 / 16.0, TsumoPossibility::possibility(PuyoSet(0, 0, 2, 0), 2));
    EXPECT_EQ(1.0 / 16.0, TsumoPossibility::possibility(PuyoSet(0, 0, 0, 2), 2));

    EXPECT_EQ(2.0 / 16.0, TsumoPossibility::possibility(PuyoSet(1, 1, 0, 0), 2));
    EXPECT_EQ(2.0 / 16.0, TsumoPossibility::possibility(PuyoSet(1, 0, 1, 0), 2));
    EXPECT_EQ(2.0 / 16.0, TsumoPossibility::possibility(PuyoSet(1, 0, 0, 1), 2));
    EXPECT_EQ(2.0 / 16.0, TsumoPossibility::possibility(PuyoSet(0, 1, 1, 0), 2));
    EXPECT_EQ(2.0 / 16.0, TsumoPossibility::possibility(PuyoSet(0, 1, 0, 1), 2));
    EXPECT_EQ(2.0 / 16.0, TsumoPossibility::possibility(PuyoSet(0, 0, 1, 1), 2));

    // (5!/2!) / 4^5
    EXPECT_EQ(15.0 / 256.0, TsumoPossibility::possibility(PuyoSet(2, 1, 1, 1), 5));

    // RGBY-R or RGBY-G or RGBY-B or RGBY-Y
    // 4 * (5!/2!) / 4^5
    EXPECT_EQ(15.0 / 64.0, TsumoPossibility::possibility(PuyoSet(1, 1, 1, 1), 5));
}

TEST(TsumoPossibilityTest, necessaryPuyos)
{
    TsumoPossibility::initialize();

    EXPECT_EQ(0, TsumoPossibility::necessaryPuyos(PuyoSet(0, 0, 0, 0), 1.0));
    EXPECT_EQ(0, TsumoPossibility::necessaryPuyos(PuyoSet(0, 0, 0, 0), 0.8));

    EXPECT_EQ(1, TsumoPossibility::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.249));
    EXPECT_EQ(1, TsumoPossibility::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.25));
    EXPECT_EQ(2, TsumoPossibility::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.251));

    EXPECT_EQ(1, TsumoPossibility::necessaryPuyos(PuyoSet(0, 1, 0, 0), 0.25));
    EXPECT_EQ(1, TsumoPossibility::necessaryPuyos(PuyoSet(0, 0, 1, 0), 0.25));
    EXPECT_EQ(1, TsumoPossibility::necessaryPuyos(PuyoSet(0, 0, 0, 1), 0.25));

    EXPECT_EQ(1, TsumoPossibility::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.2));
    EXPECT_EQ(4, TsumoPossibility::necessaryPuyos(PuyoSet(2, 0, 0, 0), 0.2));
    EXPECT_EQ(7, TsumoPossibility::necessaryPuyos(PuyoSet(3, 0, 0, 0), 0.2));
    EXPECT_EQ(3, TsumoPossibility::necessaryPuyos(PuyoSet(1, 1, 0, 0), 0.2));
    EXPECT_EQ(7, TsumoPossibility::necessaryPuyos(PuyoSet(2, 2, 0, 0), 0.2));

    EXPECT_EQ(3, TsumoPossibility::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.5));
    EXPECT_EQ(7, TsumoPossibility::necessaryPuyos(PuyoSet(2, 0, 0, 0), 0.5));
    EXPECT_EQ(11, TsumoPossibility::necessaryPuyos(PuyoSet(3, 0, 0, 0), 0.5));
    EXPECT_EQ(5, TsumoPossibility::necessaryPuyos(PuyoSet(1, 1, 0, 0), 0.5));
    EXPECT_EQ(10, TsumoPossibility::necessaryPuyos(PuyoSet(2, 2, 0, 0), 0.5));

    EXPECT_EQ(6, TsumoPossibility::necessaryPuyos(PuyoSet(1, 0, 0, 0), 0.8));
    EXPECT_EQ(11, TsumoPossibility::necessaryPuyos(PuyoSet(2, 0, 0, 0), 0.8));
    EXPECT_EQ(16, TsumoPossibility::necessaryPuyos(PuyoSet(3, 0, 0, 0), 0.8));
    EXPECT_EQ(8, TsumoPossibility::necessaryPuyos(PuyoSet(1, 1, 0, 0), 0.8));
    EXPECT_EQ(14, TsumoPossibility::necessaryPuyos(PuyoSet(2, 2, 0, 0), 0.8));
}
