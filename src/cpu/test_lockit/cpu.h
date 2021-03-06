#ifndef TEST_LOCKIT_CPU_RUN_H_
#define TEST_LOCKIT_CPU_RUN_H_

#include "core/client/ai/raw_ai.h"
#include "read.h"
#include "coma.h"

class TestLockitAI : public RawAI {
public:
    TestLockitAI();

protected:
    FrameResponse playOneFrame(const FrameRequest&) override;

private:
    COMAI_HI coma;
    READ_P r_player[2];
};

#endif
