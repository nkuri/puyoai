#include "core/algorithm/field_pattern.h"

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "core/column_puyo.h"
#include "core/column_puyo_list.h"
#include "core/core_field.h"
#include "core/field_bit_field.h"
#include "core/position.h"

using namespace std;

TEST(FieldPatternTest, initial)
{
    FieldPattern pattern;
    for (int i = 1; i <= 6; ++i) {
        EXPECT_EQ(0, pattern.height(i));
    }
}

TEST(FieldPatternTest, constructor1)
{
    FieldPattern pattern(
        ".....B"
        "....BB"
        "...BBB"
        "..BBBB"
        ".BBBBB"
        "AAABBC");

    EXPECT_EQ(1, pattern.height(1));
    EXPECT_EQ(2, pattern.height(2));
    EXPECT_EQ(3, pattern.height(3));
    EXPECT_EQ(4, pattern.height(4));
    EXPECT_EQ(5, pattern.height(5));
    EXPECT_EQ(6, pattern.height(6));

    EXPECT_EQ('A', pattern.variable(1, 1));
    EXPECT_EQ('B', pattern.variable(2, 2));
    EXPECT_EQ('C', pattern.variable(6, 1));
    EXPECT_EQ(' ', pattern.variable(1, 2));
}

TEST(FieldPatternTest, constructor2)
{
    FieldPattern pattern(vector<string> {
        ".....B",
        "....BB",
        "...BBB",
        "..BBBB",
        ".BBBBB",
        "AAABBC"
    });

    EXPECT_EQ(1, pattern.height(1));
    EXPECT_EQ(2, pattern.height(2));
    EXPECT_EQ(3, pattern.height(3));
    EXPECT_EQ(4, pattern.height(4));
    EXPECT_EQ(5, pattern.height(5));
    EXPECT_EQ(6, pattern.height(6));

    EXPECT_EQ('A', pattern.variable(1, 1));
    EXPECT_EQ('B', pattern.variable(2, 2));
    EXPECT_EQ('C', pattern.variable(6, 1));
    EXPECT_EQ(' ', pattern.variable(1, 2));
}

TEST(FieldPatternTest, varCount)
{
    FieldPattern pattern1("AAA...");
    FieldPattern pattern2("..AAAB");
    FieldPattern pattern3(".*ABBB");

    EXPECT_EQ(3, pattern1.numVariables());
    EXPECT_EQ(4, pattern2.numVariables());
    EXPECT_EQ(4, pattern3.numVariables()); // We don't count *

    {
        FieldPattern pattern;
        ASSERT_TRUE(FieldPattern::merge(pattern1, pattern2, &pattern));
        EXPECT_EQ(6, pattern.numVariables());
    }
    {
        FieldPattern pattern;
        ASSERT_TRUE(FieldPattern::merge(pattern1, pattern2, &pattern));
        EXPECT_EQ(6, pattern.numVariables());
    }
}

TEST(FieldPatternTest, fillSameVariablePositions)
{
    FieldPattern pattern(
        "C....."
        "CAA..."
        "CCDAAA"
        "AAADDD");

    FieldBitField checked;
    Position positionQueue[FieldConstant::WIDTH * FieldConstant::HEIGHT];
    Position* p = pattern.fillSameVariablePositions(1, 2, 'C', positionQueue, &checked);
    EXPECT_EQ(4, p - positionQueue);

    std::sort(positionQueue, p);
    EXPECT_EQ(Position(1, 2), positionQueue[0]);
    EXPECT_EQ(Position(1, 3), positionQueue[1]);
    EXPECT_EQ(Position(1, 4), positionQueue[2]);
    EXPECT_EQ(Position(2, 2), positionQueue[3]);
}

TEST(FieldPatternTest, complement1)
{
    FieldPattern pattern(
        "..BC.."
        "AAAB.."
        "BBBCCC");

    CoreField cf(
        "......"
        "..YB.."
        "BBBG..");

    CoreField expected(
        "..BG.."
        "YYYB.."
        "BBBGGG");

    ColumnPuyoList cpl;
    EXPECT_TRUE(pattern.complement(cf, &cpl).success);
    EXPECT_TRUE(cf.dropPuyoList(cpl));
    EXPECT_EQ(expected, cf) << cf.toDebugString();
}

TEST(FieldPatternTest, complement2)
{
    FieldPattern pattern(
        "B....."
        "AAA..."
        "BC...."
        "BBC..."
        "CC....");

    CoreField cf(
        "YB...."
        "YYB..."
        "BBGGG.");

    // Since we cannot complement (3, 3), so this pattern should not match.
    ColumnPuyoList cpl;
    EXPECT_FALSE(pattern.complement(cf, &cpl).success);
}

TEST(FieldPatternTest, complement3)
{
    FieldPattern pattern(
        "BA...."
        "AA...."
        "BC...."
        "BBC..."
        "CC....");

    CoreField cf(
        " R    "
        "YY BBB"
        "RRRGGG");

    ColumnPuyoList cpl;
    EXPECT_FALSE(pattern.complement(cf, &cpl).success);
}

TEST(FieldPatternTest, complement4)
{
    FieldPattern pattern(
        "....De"
        "ABCDDE"
        "AABCCD"
        "BBCEEE");

    CoreField cf(
        ".....Y"
        "Y....Y"
        "Y..RRB"
        "GG.YYY");

    ColumnPuyoList cpl;
    EXPECT_TRUE(pattern.complement(cf, &cpl).success);
}

TEST(FieldPatternTest, complement5)
{
    FieldPattern pattern(
        "ABC..."
        "AABCC."
        "BBC@@.");

    EXPECT_EQ(PatternType::ALLOW_FILLING_OJAMA, pattern.type(4, 1));

    CoreField cf(
        "Y....."
        "Y....."
        "GGB...");

    CoreField expected(
        "YGB..."
        "YYGBB."
        "GGBOO.");

    ColumnPuyoList cpl;
    EXPECT_TRUE(pattern.complement(cf, &cpl).success);
    EXPECT_TRUE(cf.dropPuyoList(cpl));
    EXPECT_EQ(expected, cf) << expected.toDebugString() << '\n' << cf.toDebugString();
}

TEST(FieldPatternTest, complement6)
{
    FieldPattern pattern(
        "ABC..."
        "AABCC."
        "BBC@@.");

    EXPECT_EQ(PatternType::ALLOW_FILLING_OJAMA, pattern.type(4, 1));

    CoreField cf(
        "Y....."
        "Y....."
        "GG....");

    CoreField expected1(
        "YGB..."
        "YYGBB."
        "GGBOO.");

    CoreField expected2(
        "YGR..."
        "YYGRR."
        "GGROO.");

    ColumnPuyoList cpl;
    EXPECT_TRUE(pattern.complement(cf, 1, &cpl).success);
    EXPECT_TRUE(cf.dropPuyoList(cpl));
    EXPECT_TRUE(cf == expected1 || cf == expected2) << cf.toDebugString();
}

TEST(FieldPatternTest, complementWithAllow1)
{
    FieldPattern pattern(
        ".ab..."
        ".AB..."
        "ABC..."
        "ABC..."
        "ABCC..");

    CoreField cf(
        "YGB..."
        "YGB..."
        "YGB...");

    CoreField expected(
        ".YG..."
        "YGB..."
        "YGB..."
        "YGBB..");

    ColumnPuyoList cpl;
    EXPECT_TRUE(pattern.complement(cf, 1, &cpl).success);
    EXPECT_TRUE(cf.dropPuyoList(cpl));
    EXPECT_TRUE(expected == cf);
}

TEST(FieldPatternTest, complementWithAllow2)
{
    FieldPattern pattern(
        ".ab..."
        ".AB..."
        "ABC..."
        "ABC..."
        "ABCC..");

    CoreField cf(
        ".Y...."
        ".Y...."
        "YGB..."
        "YGB..."
        "YGB...");

    CoreField expected(
        ".Y...."
        ".YG..."
        "YGB..."
        "YGB..."
        "YGBB..");


    ColumnPuyoList cpl;
    EXPECT_TRUE(pattern.complement(cf, 1, &cpl).success);
    EXPECT_TRUE(cf.dropPuyoList(cpl));
    EXPECT_TRUE(expected == cf);
}
