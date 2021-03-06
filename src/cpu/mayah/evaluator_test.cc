#include "evaluator.h"

#include <algorithm>
#include <gtest/gtest.h>

#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/decision.h"
#include "core/core_field.h"
#include "gazer.h"

using namespace std;

class EvaluatorTest : public testing::Test {
protected:
    CollectedFeature eval(const CoreField& f, int numIteration = 1)
    {
        TsumoPossibility::initialize();

        EvaluationParameter evaluationParameter;
        PatternBook patternBook;
        Gazer gazer;

        gazer.initialize(100);

        vector<Decision> decisions { Decision(3, 0) };
        RensaResult rensaResult;
        int framesToIgnite = 10;
        int lastDropFrames = 10;

        RefPlan plan(f, decisions, rensaResult, 0, framesToIgnite, lastDropFrames);

        PreEvalResult preEvalResult = PreEvaluator(patternBook).preEval(f);
        FeatureScoreCollector sc(evaluationParameter);
        Evaluator<FeatureScoreCollector> evaluator(patternBook, &sc);
        evaluator.collectScore(plan, f, 1, numIteration, PlayerState(), PlayerState(), preEvalResult, MidEvalResult(), gazer.gazeResult());
        return sc.toCollectedFeature();
    }

    template<typename F>
    CollectedFeature withEvaluator(F f) {
        TsumoPossibility::initialize();

        EvaluationParameter evaluationParameter;
        PatternBook patternBook;
        FeatureScoreCollector sc(evaluationParameter);
        Evaluator<FeatureScoreCollector> evaluator(patternBook, &sc);

        f(&evaluator);

        return sc.toCollectedFeature();
    }

    template<typename F>
    CollectedFeature withRensaEvaluator(F f) {
        TsumoPossibility::initialize();

        EvaluationParameter evaluationParameter;
        PatternBook patternBook;
        FeatureScoreCollector sc(evaluationParameter);
        RensaEvaluator<FeatureScoreCollector> rensaEvaluator(patternBook, &sc);

        f(&rensaEvaluator);

        return sc.toCollectedFeature();
    }
};

TEST_F(EvaluatorTest, collectScoreForRensaGarbage)
{
    CoreField f("R    R"
                "R    R"
                "YYYGGG");

    EvaluationParameter param;
    PatternBook patternBook;
    FeatureScoreCollector sc(param);
    RensaEvaluator<FeatureScoreCollector> evaluator(patternBook, &sc);

    evaluator.collectScoreForRensaGarbage(f);
    CollectedFeature cf = sc.toCollectedFeature();

    EXPECT_EQ(10, cf.feature(NUM_GARBAGE_PUYOS));
    EXPECT_EQ(6, cf.feature(NUM_SIDE_GARBAGE_PUYOS));
}

TEST_F(EvaluatorTest, RidgeHeight1)
{
    CoreField f(
        "  O   "
        "  O   "
        "  O   "
        "  O  O"
        "OOOOOO");

    CollectedFeature cf = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalRidgeHeight(f);
    });

    const vector<int>& vs = cf.feature(RIDGE_HEIGHT);
    EXPECT_TRUE(find(vs.begin(), vs.end(), 4) != vs.end());
}

TEST_F(EvaluatorTest, RidgeHeight2)
{
    CoreField f(
        "  O   "
        "  O   "
        "  OO  "
        " OOO O"
        "OOOOOO");

    CollectedFeature cf = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalRidgeHeight(f);
    });

    const vector<int>& vs = cf.feature(RIDGE_HEIGHT);
    EXPECT_TRUE(find(vs.begin(), vs.end(), 2) != vs.end());
}

TEST_F(EvaluatorTest, RidgeHeight3)
{
    CoreField f(
        "  O   "
        " OO   "
        " OO   "
        " OOO O"
        "OOOOOO");

    CollectedFeature cf = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalRidgeHeight(f);
    });

    const vector<int>& vs = cf.feature(RIDGE_HEIGHT);
    EXPECT_TRUE(find(vs.begin(), vs.end(), 1) != vs.end());
}

TEST_F(EvaluatorTest, connection)
{
    CoreField f("BBBYYY"
                "OOOOGO"
                "BBYYGO");

    CollectedFeature cf = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->collectScoreForConnection(f);
    });

    EXPECT_EQ(2, cf.feature(CONNECTION_3));
    EXPECT_EQ(3, cf.feature(CONNECTION_2));
}

TEST_F(EvaluatorTest, connectionHorizontal)
{
    CoreField f(
        "OGGGOO"
        "OOYYOO"
        "RRROGG");

    CollectedFeature cf = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalRestrictedConnectionHorizontalFeature(f);
    });

    EXPECT_EQ(1, cf.feature(CONNECTION_HORIZONTAL_2));
    EXPECT_EQ(1, cf.feature(CONNECTION_HORIZONTAL_3));
    EXPECT_EQ(1, cf.feature(CONNECTION_HORIZONTAL_CROSSED_2));
    EXPECT_EQ(1, cf.feature(CONNECTION_HORIZONTAL_CROSSED_3));
}

TEST_F(EvaluatorTest, NumUnreachableSpace1)
{
    CoreField f(
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    CollectedFeature cf = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalUnreachableSpace(f);
    });

    EXPECT_EQ(0, cf.feature(NUM_UNREACHABLE_SPACE));
}

TEST_F(EvaluatorTest, NumUnreachableSpace2)
{
    CoreField f(
        "    O " // 12
        "    O "
        "    O "
        "    O "
        "    O " // 8
        "    O "
        "    O "
        "    O "
        "    O " // 4
        "    O "
        "    O "
        "    O ");

    CollectedFeature cf = withEvaluator([&f](Evaluator<FeatureScoreCollector>* evaluator) {
        evaluator->evalUnreachableSpace(f);
    });

    EXPECT_EQ(12, cf.feature(NUM_UNREACHABLE_SPACE));
}

TEST_F(EvaluatorTest, sideChain)
{
    CoreField f(
        ".R...."
        "RB...."
        "RB...."
        "RB...."
        "BR...."
        "BR...."
        "BRGYG."
        "RGYGB."
        "RGYGB."
        "RGYGB.");

    CollectedFeature cf = eval(f, 1);
    EXPECT_EQ(1.0, cf.feature(HOLDING_SIDE_CHAIN_MEDIUM));
}

TEST_F(EvaluatorTest, DontCrash1)
{
    CoreField f(
        "@@  @@"
        "@@  @@"
        "@@@@@@"
        "@@@@@@"
        "@@@@@@"
        "@@@@YG"
        "@@@@@@"
        "@@@@@@"
        "BG@@@@"
        "YY@@@@"
        "BG@@@@"
        "GG@@RY"
        "BBBRRB");
    (void)eval(f);
}

TEST_F(EvaluatorTest, DontCrash2)
{
    CoreField f;
    (void)eval(f);
}
