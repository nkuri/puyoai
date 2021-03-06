#include "core/core_field.h"

#include <array>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "core/column_puyo.h"
#include "core/column_puyo_list.h"
#include "core/constant.h"
#include "core/decision.h"
#include "core/field_bit_field.h"
#include "core/kumipuyo.h"
#include "core/position.h"
#include "core/rensa_result.h"
#include "core/score.h"

using namespace std;

namespace {
int g_originalY[FieldConstant::MAP_WIDTH][FieldConstant::MAP_HEIGHT];

bool initializeOriginalY()
{
    for (int x = 0; x < FieldConstant::MAP_WIDTH; ++x) {
        for (int y = 0; y < FieldConstant::MAP_HEIGHT; ++y) {
            g_originalY[x][y] = y;
        }
    }

    return true;
}

static bool g_originalYIsInitialized = initializeOriginalY();
}

class RensaNonTracker {
public:
    void colorPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) { }
    void ojamaPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) { }
    void puyoIsDropped(int /*x*/, int /*fromY*/, int /*toY*/) { }
    void nthChainDone(int /*nthChain*/, int /*numErasedPuyo*/, int /*coef*/) {}
};

class RensaTracker {
public:
    RensaTracker(RensaTrackResult* trackResult) :
        result_(trackResult)
    {
        // TODO(mayah): Assert trackResult is initialized.
        DCHECK(g_originalYIsInitialized);
        static_assert(sizeof(originalY_ == g_originalY), "originalY_ and g_originalY should have the same size.");
        memcpy(originalY_, g_originalY, sizeof(originalY_));
    }

    void colorPuyoIsVanished(int x, int y, int nthChain) { result_->setErasedAt(x, originalY_[x][y], nthChain); }
    void ojamaPuyoIsVanished(int x, int y, int nthChain) { result_->setErasedAt(x, originalY_[x][y], nthChain); }
    void puyoIsDropped(int x, int fromY, int toY) { originalY_[x][toY] = originalY_[x][fromY]; }
    void nthChainDone(int /*nthChain*/, int /*numErasedPuyo*/, int /*coef*/) {}

private:
    int originalY_[FieldConstant::MAP_WIDTH][FieldConstant::MAP_HEIGHT];
    RensaTrackResult* result_;
};

class RensaCoefTracker {
public:
    explicit RensaCoefTracker(RensaCoefResult* result) : result_(result) {}

    void colorPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) {}
    void ojamaPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) {}
    void puyoIsDropped(int /*x*/, int /*fromY*/, int /*toY*/) {}
    void nthChainDone(int nthChain, int numErasedPuyo, int coef) { result_->setCoef(nthChain, numErasedPuyo, coef); }

private:
    RensaCoefResult* result_;
};

class RensaVanishingPositionTracker {
public:
    RensaVanishingPositionTracker(RensaVanishingPositionResult* result) : result_(result)
    {
        resetY();
    }

    void colorPuyoIsVanished(int x, int y, int nthChain)
    {
        if (yAtPrevRensa_[x][y] == 0) {
            result_->setBasePuyo(x, y, nthChain);
        } else {
            result_->setFallingPuyo(x, yAtPrevRensa_[x][y], y, nthChain);
        }
    }

    void ojamaPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) {}
    void puyoIsDropped(int x, int fromY, int toY) { yAtPrevRensa_[x][toY] = fromY; }
    void nthChainDone(int /*nthChain*/, int /*numErasedPuyo*/, int /*coef*/) { resetY(); }

private:
    void resetY() {
        constexpr std::array<int, FieldConstant::MAP_HEIGHT> ALL_ZERO {{}};
        yAtPrevRensa_.fill(ALL_ZERO);
    }

    RensaVanishingPositionResult* result_;
    std::array<std::array<int, FieldConstant::MAP_HEIGHT>, FieldConstant::MAP_WIDTH> yAtPrevRensa_;
};

CoreField::CoreField()
{
    for (int x = 0; x < MAP_WIDTH; ++x)
        heights_[x] = 0;
}

CoreField::CoreField(const std::string& url) :
    PlainField(url)
{
    heights_[0] = 0;
    heights_[MAP_WIDTH - 1] = 0;
    for (int x = 1; x <= WIDTH; ++x)
        recalcHeightOn(x);
}

CoreField::CoreField(const PlainField& f) :
    PlainField(f)
{
    heights_[0] = 0;
    heights_[MAP_WIDTH - 1] = 0;
    for (int x = 1; x <= WIDTH; ++x)
        recalcHeightOn(x);
}

void CoreField::clear()
{
    *this = CoreField();
}

bool CoreField::isZenkeshi() const
{
    for (int x = 1; x <= WIDTH; ++x) {
        if (color(x, 1) != PuyoColor::EMPTY)
            return false;
    }

    return true;
}

bool CoreField::isZenkeshiPrecise() const
{
    for (int x = 1; x <= WIDTH; ++x) {
        for (int y = 1; y <= 13; ++y) {
            if (color(x, y) != PuyoColor::EMPTY)
                return false;
        }
    }

    return true;
}

int CoreField::countPuyos() const
{
    int count = 0;
    for (int x = 1; x <= WIDTH; ++x)
        count += height(x);

    return count;
}

int CoreField::countColorPuyos() const
{
    int cnt = 0;
    for (int x = 1; x <= WIDTH; ++x) {
        for (int y = 1; y <= height(x); ++y) {
            if (isNormalColor(color(x, y)))
                ++cnt;
        }
    }

    return cnt;
}

int CoreField::countConnectedPuyos(int x, int y) const
{
    FieldBitField checked;
    return countConnectedPuyos(x, y, &checked);
}

int CoreField::countConnectedPuyos(int x, int y, FieldBitField* checked) const
{
    Position positions[WIDTH * HEIGHT];

    Position* filledHead = fillSameColorPosition(x, y, color(x, y), positions, checked);
    return filledHead - positions;
}

int CoreField::countConnectedPuyosMax4(int x, int y) const
{
    bool leftUp = false, leftDown = false, rightUp = false, rightDown = false;
    int cnt = 1;
    PuyoColor c = color(x, y);

    if (color(x - 1, y) == c) {
        if (color(x - 2, y) == c) {
            if (color(x - 3, y) == c || (color(x - 2, y + 1) == c && y + 1 <= 12) || color(x - 2, y - 1) == c)
                return 4;
            ++cnt;
        }
        if (color(x - 1, y + 1) == c && y + 1 <= 12) {
            if (color(x - 2, y + 1) == c || (color(x - 1, y + 2) == c && y + 2 <= 12))
                return 4;
            ++cnt;
            leftUp = true;
        }
        if (color(x - 1, y - 1) == c) {
            if (color(x - 2, y - 1) == c || color(x - 1, y - 2) == c)
                return 4;
            ++cnt;
            leftDown = true;
        }
        ++cnt;
    }
    if (color(x + 1, y) == c) {
        if (color(x + 2, y) == c) {
            if (color(x + 3, y) == c || (color(x + 2, y + 1) == c && y + 1 <= 12) || color(x + 2, y - 1) == c)
                return 4;
            ++cnt;
        }
        if (color(x + 1, y + 1) == c && y + 1 <= 12) {
            if (color(x + 2, y + 1) == c || (color(x + 1, y + 2) == c && y + 2 <= 12))
                return 4;
            ++cnt;
            rightUp = true;
        }
        if (color(x + 1, y - 1) == c) {
            if (color(x + 2, y - 1) == c || color(x + 1, y - 2) == c)
                return 4;
            ++cnt;
            rightDown = true;
        }
        ++cnt;
    }
    if (color(x, y - 1) == c) {
        if (color(x, y - 2) == c) {
            if (color(x, y - 3) == c || color(x - 1, y - 2) == c || color(x + 1, y - 2) == c)
                return 4;
            ++cnt;
        }
        if (color(x - 1, y - 1) == c && !leftDown) {
            if (color(x - 2, y - 1) == c || color(x - 1, y - 2) == c)
                return 4;
            ++cnt;
        }
        if (color(x + 1, y - 1) == c && !rightDown) {
            if (color(x + 2, y - 1) == c || color(x + 1, y - 2) == c)
                return 4;
            ++cnt;
        }
        ++cnt;
    }
    if (color(x, y + 1) == c && y + 1 <= 12) {
        if (color(x, y + 2) == c && y + 2 <= 12) {
            if ((color(x, y + 3) == c && y + 3 <= 12) || color(x - 1, y + 2) == c || color(x + 1, y + 2) == c)
                return 4;
            ++cnt;
        }
        if (color(x - 1, y + 1) == c && !leftUp) {
            if (color(x - 2, y + 1) == c || (color(x - 1, y + 2) == c && y + 2 <= 12))
                return 4;
            ++cnt;
        }
        if (color(x + 1, y + 1) == c && !rightUp) {
            if (color(x + 2, y + 1) == c || (color(x + 1, y + 2) == c && y + 2 <= 12))
                return 4;
            ++cnt;
        }
        ++cnt;
    }

    return (cnt >= 5) ? 4 : cnt;
}

bool CoreField::isConnectedPuyo(int x, int y) const
{
    PuyoColor c = color(x, y);
    return color(x, y - 1) == c || color(x, y + 1) == c || color(x - 1, y) == c || color(x + 1, y) == c;
}

bool CoreField::hasEmptyNeighbor(int x, int y) const
{
    DCHECK(color(x, y) != PuyoColor::EMPTY);
    return color(x, y + 1) == PuyoColor::EMPTY ||
           color(x, y - 1) == PuyoColor::EMPTY ||
           color(x + 1, y) == PuyoColor::EMPTY ||
           color(x - 1, y) == PuyoColor::EMPTY;
}

void CoreField::forceDrop()
{
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        int writeYAt = 1;
        for (int y = 1; y <= 13; ++y) {
            if (color(x, y) != PuyoColor::EMPTY)
                unsafeSet(x, writeYAt++, color(x, y));
        }
        for (int y = writeYAt; y <= 13; ++y)
            unsafeSet(x, y, PuyoColor::EMPTY);
        heights_[x] = writeYAt - 1;
    }
}

bool CoreField::dropKumipuyo(const Decision& decision, const Kumipuyo& kumiPuyo)
{
    int x1 = decision.axisX();
    int x2 = decision.childX();
    PuyoColor c1 = kumiPuyo.axis;
    PuyoColor c2 = kumiPuyo.child;

    if (decision.r == 2) {
        if (!dropPuyoOnWithMaxHeight(x2, c2, 14))
            return false;
        if (!dropPuyoOnWithMaxHeight(x1, c1, 13)) {
            removeTopPuyoFrom(x2);
            return false;
        }
        return true;
    }

    if (!dropPuyoOnWithMaxHeight(x1, c1, 13))
        return false;
    if (!dropPuyoOnWithMaxHeight(x2, c2, 14)) {
        removeTopPuyoFrom(x1);
        return false;
    }
    return true;
}

void CoreField::undoKumipuyo(const Decision& decision)
{
    removeTopPuyoFrom(decision.x);
    removeTopPuyoFrom(decision.childX());
}

KumipuyoPos CoreField::dropPosition(const Decision& decision) const
{
    int x1 = decision.axisX();
    int x2 = decision.childX();

    if (decision.r == 2)
        return KumipuyoPos(x1, height(x1) + 2, decision.rot());

    int y = std::max(height(x1), height(x2)) + 1;
    return KumipuyoPos(x1, y, decision.rot());
}

int CoreField::framesToDropNext(const Decision& decision) const
{
    // TODO(mayah): This calculation should be more accurate. We need to compare this with
    // actual AC puyo2 and duel server algorithm. These must be much the same.

    // TODO(mayah): When "kabegoe" happens, we need more frames.
    const int KABEGOE_PENALTY = 6;

    // TODO(mayah): It looks drop animation is too short.

    int x1 = decision.axisX();
    int x2 = decision.childX();

    int dropFrames = FRAMES_TO_MOVE_HORIZONTALLY[abs(3 - x1)];

    if (decision.r == 0) {
        int dropHeight = HEIGHT - height(x1);
        if (dropHeight <= 0) {
            // TODO(mayah): We need to add penalty here. How much penalty is necessary?
            dropFrames += KABEGOE_PENALTY + FRAMES_GROUNDING;
        } else {
            dropFrames += FRAMES_TO_DROP_FAST[dropHeight] + FRAMES_GROUNDING;
        }
    } else if (decision.r == 2) {
        int dropHeight = HEIGHT - height(x1) - 1;
        // TODO: If puyo lines are high enough, rotation might take time. We should measure this later.
        // It looks we need 3 frames to waiting that each rotation has completed.
        if (dropHeight < 6)
            dropHeight = 6;

        dropFrames += FRAMES_TO_DROP_FAST[dropHeight] + FRAMES_GROUNDING;
    } else {
        if (height(x1) == height(x2)) {
            int dropHeight = HEIGHT - height(x1);
            if (dropHeight <= 0) {
                dropFrames += KABEGOE_PENALTY + FRAMES_GROUNDING;
            } else if (dropHeight < 3) {
                dropFrames += FRAMES_TO_DROP_FAST[3] + FRAMES_GROUNDING;
            } else {
                dropFrames += FRAMES_TO_DROP_FAST[dropHeight] + FRAMES_GROUNDING;
            }
        } else {
            int minHeight = min(height(x1), height(x2));
            int maxHeight = max(height(x1), height(x2));
            int diffHeight = maxHeight - minHeight;
            int dropHeight = HEIGHT - maxHeight;
            if (dropHeight <= 0) {
                dropFrames += KABEGOE_PENALTY;
            } else if (dropHeight < 3) {
                dropFrames += FRAMES_TO_DROP_FAST[3];
            } else {
                dropFrames += FRAMES_TO_DROP_FAST[dropHeight];
            }
            dropFrames += FRAMES_GROUNDING;
            dropFrames += FRAMES_TO_DROP[diffHeight];
            dropFrames += FRAMES_GROUNDING;
        }
    }

    CHECK(dropFrames >= 0);
    return dropFrames;
}

bool CoreField::isChigiriDecision(const Decision& decision) const
{
    DCHECK(decision.isValid()) << "decision " << decision.toString() << " should be valid.";

    if (decision.axisX() == decision.childX())
        return false;

    return height(decision.axisX()) != height(decision.childX());
}

bool CoreField::dropPuyoOnWithMaxHeight(int x, PuyoColor c, int maxHeight)
{
    DCHECK_NE(c, PuyoColor::EMPTY) << toDebugString();
    DCHECK_LE(maxHeight, 14);

    if (height(x) >= maxHeight)
        return false;

    DCHECK_EQ(color(x, height(x) + 1), PuyoColor::EMPTY);
    unsafeSet(x, ++heights_[x], c);
    return true;
}

bool CoreField::dropPuyoListWithMaxHeight(const ColumnPuyoList& cpl, int maxHeight)
{
    for (int x = 1; x <= 6; ++x) {
        int s = cpl.sizeOn(x);
        for (int i = 0; i < s; ++i) {
            PuyoColor pc = cpl.get(x, i);
            if (!dropPuyoOnWithMaxHeight(x, pc, maxHeight))
                return false;
        }
    }

    return true;
}

Position* CoreField::fillSameColorPosition(int x, int y, PuyoColor c,
                                           Position* positionQueueHead, FieldBitField* checked) const
{
    DCHECK(!checked->get(x, y));

    if (FieldConstant::HEIGHT < y)
        return positionQueueHead;

    Position* writeHead = positionQueueHead;
    Position* readHead = positionQueueHead;

    *writeHead++ = Position(x, y);
    checked->set(x, y);

    while (readHead != writeHead) {
        Position p = *readHead++;

        if (color(p.x + 1, p.y) == c && !checked->get(p.x + 1, p.y)) {
            *writeHead++ = Position(p.x + 1, p.y);
            checked->set(p.x + 1, p.y);
        }
        if (color(p.x - 1, p.y) == c && !checked->get(p.x - 1, p.y)) {
            *writeHead++ = Position(p.x - 1, p.y);
            checked->set(p.x - 1, p.y);
        }
        if (color(p.x, p.y + 1) == c && !checked->get(p.x, p.y + 1) && p.y + 1 <= FieldConstant::HEIGHT) {
            *writeHead++ = Position(p.x, p.y + 1);
            checked->set(p.x, p.y + 1);
        }
        if (color(p.x, p.y - 1) == c && !checked->get(p.x, p.y - 1)) {
            *writeHead++ = Position(p.x, p.y - 1);
            checked->set(p.x, p.y - 1);
        }
    }

    return writeHead;
}

int CoreField::vanishOnly(int currentNthChain)
{
    SimulationContext context(currentNthChain);
    RensaNonTracker nonTracker;
    return vanish(&context, &nonTracker);
}

int CoreField::vanishDrop(SimulationContext* context)
{
    RensaNonTracker tracker;
    int score = vanish(context, &tracker);
    if (score > 0)
        dropAfterVanish(context, &tracker);

    return score;
}

template<typename Tracker>
int CoreField::vanish(SimulationContext* context, Tracker* tracker)
{
    FieldBitField checked;
    Position eraseQueue[WIDTH * HEIGHT]; // All the positions of erased puyos will be stored here.
    Position* eraseQueueHead = eraseQueue;

    bool usedColors[NUM_PUYO_COLORS] {};
    int numUsedColors = 0;
    int longBonusCoef = 0;

    for (int x = 1; x <= WIDTH; ++x) {
        int maxHeight = height(x);
        for (int y = context->minHeights[x]; y <= maxHeight; ++y) {
            DCHECK_NE(color(x, y), PuyoColor::EMPTY)
                << x << ' ' << y << ' ' << toChar(color(x, y)) << '\n'
                << toDebugString();

            if (checked.get(x, y))
                continue;
            if (!isNormalColor(color(x, y)))
                continue;

            PuyoColor c = color(x, y);
            Position* head = fillSameColorPosition(x, y, c, eraseQueueHead, &checked);

            int connectedPuyoNum = head - eraseQueueHead;
            if (connectedPuyoNum < PUYO_ERASE_NUM)
                continue;

            eraseQueueHead = head;
            longBonusCoef += longBonus(connectedPuyoNum);
            if (!usedColors[static_cast<int>(c)]) {
                ++numUsedColors;
                usedColors[static_cast<int>(c)] = true;
            }
        }
    }

    int numErasedPuyos = eraseQueueHead - eraseQueue;
    if (numErasedPuyos == 0)
        return 0;

    // --- Actually erase the Puyos to be vanished. We erase ojama here also.
    eraseQueuedPuyos(context, eraseQueue, eraseQueueHead, tracker);

    int rensaBonusCoef = calculateRensaBonusCoef(chainBonus(context->currentChain), longBonusCoef, colorBonus(numUsedColors));
    tracker->nthChainDone(context->currentChain, numErasedPuyos, rensaBonusCoef);
    return 10 * numErasedPuyos * rensaBonusCoef;
}

template<typename Tracker>
void CoreField::eraseQueuedPuyos(SimulationContext* context, Position* eraseQueue, Position* eraseQueueHead, Tracker* tracker)
{
    DCHECK(tracker);

    context->updateFromField(*this);

    for (Position* head = eraseQueue; head != eraseQueueHead; ++head) {
        int x = head->x;
        int y = head->y;

        unsafeSet(x, y, PuyoColor::EMPTY);
        tracker->colorPuyoIsVanished(x, y, context->currentChain);
        context->minHeights[x] = std::min(context->minHeights[x], y);

        // Check OJAMA puyos erased
        if (color(x + 1, y) == PuyoColor::OJAMA) {
            unsafeSet(x + 1, y, PuyoColor::EMPTY);
            tracker->ojamaPuyoIsVanished(x + 1, y, context->currentChain);
            context->minHeights[x + 1] = std::min(context->minHeights[x + 1], y);
        }

        if (color(x - 1, y) == PuyoColor::OJAMA) {
            unsafeSet(x - 1, y, PuyoColor::EMPTY);
            tracker->ojamaPuyoIsVanished(x - 1, y, context->currentChain);
            context->minHeights[x - 1] = std::min(context->minHeights[x - 1], y);
        }

        // We don't need to update minHeights here.
        if (color(x, y + 1) == PuyoColor::OJAMA && y + 1 <= HEIGHT) {
            unsafeSet(x, y + 1, PuyoColor::EMPTY);
            tracker->ojamaPuyoIsVanished(x, y + 1, context->currentChain);
        }

        if (color(x, y - 1) == PuyoColor::OJAMA) {
            unsafeSet(x, y - 1, PuyoColor::EMPTY);
            tracker->ojamaPuyoIsVanished(x, y - 1, context->currentChain);
            context->minHeights[x] = std::min(context->minHeights[x], y - 1);
        }
    }
}

template<typename Tracker>
int CoreField::dropAfterVanish(SimulationContext* context, Tracker* tracker)
{
    DCHECK(tracker);

    int maxDrops = 0;
    for (int x = 1; x <= WIDTH; x++) {
        int writeAt = context->minHeights[x];
        if (writeAt >= 14)
            continue;

        int maxHeight = height(x);
        heights_[x] = writeAt - 1;

        DCHECK_EQ(color(x, writeAt), PuyoColor::EMPTY) << writeAt << ' ' << toChar(color(x, writeAt));
        for (int y = writeAt + 1; y <= maxHeight; ++y) {
            if (color(x, y) == PuyoColor::EMPTY)
                continue;

            maxDrops = max(maxDrops, y - writeAt);
            unsafeSet(x, writeAt, color(x, y));
            unsafeSet(x, y, PuyoColor::EMPTY);
            heights_[x] = writeAt;
            tracker->puyoIsDropped(x, y, writeAt++);
        }
    }

    return maxDrops;
}

int CoreField::fillErasingPuyoPositions(const SimulationContext& context, Position* eraseQueue) const
{
    Position* eraseQueueHead = eraseQueue;

    {
        FieldBitField checked;
        for (int x = 1; x <= WIDTH; ++x) {
            int maxHeight = height(x);
            for (int y = context.minHeights[x]; y <= maxHeight; ++y) {
                DCHECK_NE(color(x, y), PuyoColor::EMPTY)
                    << x << ' ' << y << ' ' << toChar(color(x, y)) << '\n'
                    << toDebugString();

                if (checked.get(x, y))
                    continue;
                if (!isNormalColor(color(x, y)))
                    continue;

                PuyoColor c = color(x, y);
                Position* head = fillSameColorPosition(x, y, c, eraseQueueHead, &checked);

                int connectedPuyoNum = head - eraseQueueHead;
                if (connectedPuyoNum < PUYO_ERASE_NUM)
                    continue;

                eraseQueueHead = head;
            }
        }
    }

    int numErasedPuyos = eraseQueueHead - eraseQueue;
    if (numErasedPuyos == 0)
        return 0;

    Position* colorEraseQueueHead = eraseQueueHead;

    FieldBitField checked;
    for (Position* head = eraseQueue; head != colorEraseQueueHead; ++head) {
        int x = head->x;
        int y = head->y;

        // Check OJAMA puyos erased
        if (color(x + 1, y) == PuyoColor::OJAMA && !checked(x + 1, y)) {
            checked.set(x + 1, y);
            *eraseQueueHead++ = Position(x + 1, y);
        }

        if (color(x - 1, y) == PuyoColor::OJAMA && !checked(x - 1, y)) {
            checked.set(x - 1, y);
            *eraseQueueHead++ = Position(x + 1, y);
        }

        if (color(x, y + 1) == PuyoColor::OJAMA && y + 1 <= HEIGHT && !checked(x, y + 1)) {
            checked.set(x, y + 1);
            *eraseQueueHead++ = Position(x, y + 1);
        }

        if (color(x, y - 1) == PuyoColor::OJAMA && !checked(x, y - 1)) {
            checked.set(x, y - 1);
            *eraseQueueHead++ = Position(x, y - 1);
        }
    }

    return eraseQueueHead - eraseQueue;
}

vector<Position> CoreField::erasingPuyoPositions(const SimulationContext& context) const
{
    // All the positions of erased puyos will be stored here.
    Position eraseQueue[MAP_WIDTH * MAP_HEIGHT];
    int n = fillErasingPuyoPositions(context, eraseQueue);
    return vector<Position>(eraseQueue, eraseQueue + n);
}

bool CoreField::rensaWillOccurWhenLastDecisionIs(const Decision& decision) const
{
    Position p1 = Position(decision.x, height(decision.x));
    if (countConnectedPuyos(p1.x, p1.y) >= 4)
        return true;

    Position p2;
    switch (decision.r) {
    case 0:
    case 2:
        p2 = Position(decision.x, height(decision.x) - 1);
        break;
    case 1:
        p2 = Position(decision.x + 1, height(decision.x + 1));
        break;
    case 3:
        p2 = Position(decision.x - 1, height(decision.x - 1));
        break;
    default:
        DCHECK(false) << decision.toString();
        return false;
    }

    if (countConnectedPuyos(p2.x, p2.y) >= 4)
        return true;

    return false;
}

bool CoreField::rensaWillOccurWithContext(const SimulationContext& context) const
{
    FieldBitField checked;
    for (int x = 1; x <= WIDTH; ++x) {
        int h = height(x);
        for (int y = context.minHeights[x]; y <= h; ++y) {
            if (checked.get(x, y))
                continue;

            if (!isNormalColor(color(x, y)))
                continue;

            if (countConnectedPuyos(x, y, &checked) >= 4)
                return true;
        }
    }

    return false;
}

RensaResult CoreField::simulate(int initialChain,
                                RensaTrackResult* rensaTrackResult,
                                RensaCoefResult* rensaCoefResult,
                                RensaVanishingPositionResult* rensaVanishingPositionResult)
{
    SimulationContext context(initialChain);

    if ((rensaTrackResult && rensaCoefResult)
         || (rensaCoefResult && rensaVanishingPositionResult)
         || (rensaVanishingPositionResult && rensaTrackResult)) {
        CHECK(false) << "Not supported yet";
        return RensaResult();
    } else if (rensaTrackResult) {
        RensaTracker tracker(rensaTrackResult);
        return simulateWithTracker(&context, &tracker);
    } else if (rensaCoefResult) {
        RensaCoefTracker tracker(rensaCoefResult);
        return simulateWithTracker(&context, &tracker);
    } else if (rensaVanishingPositionResult){
        RensaVanishingPositionTracker tracker(rensaVanishingPositionResult);
        return simulateWithTracker(&context, &tracker);
    } else {
        RensaNonTracker tracker;
        return simulateWithTracker(&context, &tracker);
    }
}

RensaResult CoreField::simulateWithContext(SimulationContext* context)
{
    RensaNonTracker tracker;
    return simulateWithTracker(context, &tracker);
}

RensaResult CoreField::simulateWithContext(SimulationContext* context, RensaTrackResult* rensaTrackResult)
{
    RensaTracker tracker(rensaTrackResult);
    return simulateWithTracker(context, &tracker);
}

RensaResult CoreField::simulateWithContext(SimulationContext* context, RensaCoefResult* rensaCoefResult)
{
    RensaCoefTracker tracker(rensaCoefResult);
    return simulateWithTracker(context, &tracker);
}

RensaResult CoreField::simulateWithContext(SimulationContext* context, RensaVanishingPositionResult* rensaVanishingPositionResult)
{
    RensaVanishingPositionTracker tracker(rensaVanishingPositionResult);
    return simulateWithTracker(context, &tracker);
}

template<typename Tracker>
inline RensaResult CoreField::simulateWithTracker(SimulationContext* context, Tracker* tracker)
{
    int score = 0;
    int frames = 0;

    int nthChainScore;
    bool quick = false;
    while ((nthChainScore = vanish(context, tracker)) > 0) {
        context->currentChain += 1;
        score += nthChainScore;
        frames += FRAMES_VANISH_ANIMATION;
        int maxDrops = dropAfterVanish(context, tracker);
        if (maxDrops > 0) {
            DCHECK(maxDrops < 14);
            frames += FRAMES_TO_DROP_FAST[maxDrops] + FRAMES_GROUNDING;
        } else {
            quick = true;
        }
    }

    return RensaResult(context->currentChain - 1, score, frames, quick);
}

std::string CoreField::toDebugString() const
{
    std::ostringstream s;
    for (int y = MAP_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            s << toChar(color(x, y)) << ' ';
        }
        s << std::endl;
    }
    s << ' ';
    for (int x = 1; x <= WIDTH; ++x)
        s << setw(2) << height(x);
    s << std::endl;
    return s.str();
}

// friend static
bool operator==(const CoreField& lhs, const CoreField& rhs)
{
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        if (lhs.height(x) != rhs.height(x))
            return false;

        for (int y = 1; y <= lhs.height(x); ++y) {
            if (lhs.color(x, y) != rhs.color(x, y))
                return false;
        }
    }

    return true;
}

// friend static
bool operator!=(const CoreField& lhs, const CoreField& rhs)
{
    return !(lhs == rhs);
}
