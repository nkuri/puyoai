#include "player_info.h"

#include <gtest/gtest.h>
#include "puyo.h"
#include "puyo_possibility.h"

using namespace std;

TEST(EnemyInfoTest, UpdatePossibleRensasTest)
{
    TsumoPossibility::initialize();
    
    EnemyInfo info;
    info.initializeWith(100);

    Field f("000000"
            "000000"
            "000000"
            "456000"
            "445664"
            "557774");

    vector<KumiPuyo> kumiPuyos;
    setKumiPuyo("777777", kumiPuyos);

    info.updatePossibleRensas(f, kumiPuyos);

    EXPECT_EQ(info.possibleRensaInfos().size(), 3U);
    EXPECT_EQ(info.possibleRensaInfos().back().chains, 5);
}
