#ifndef CORE_CLIENT_AI_H_
#define CORE_CLIENT_AI_H_

#include <string>

#include "core/client/ai/drop_decision.h"
#include "core/client/ai/player_state.h"
#include "core/client/connector/client_connector.h"
#include "core/kumipuyo_seq.h"

class CoreField;
class PlainField;
struct FrameRequest;

// AI is a utility class of AI.
// You need to implement think() at least.
class AI {
public:
    virtual ~AI();
    const std::string& name() const { return name_; }

    void runLoop();

protected:
    AI(int argc, char* argv[], const std::string& name);
    explicit AI(const std::string& name);

    // |think| will be called when AI should decide the next decision.
    // Basically, this will be called when NEXT2 has appeared.
    // |fast| will be true when AI should decide the next decision immeidately,
    // e.g. ojamas are dropped or the enemy has fired some rensa
    // (if you set behavior). This might be called when field is inconsistent
    // in wii_server.
    // If |fast| is true, you will have 30 ms to decide your hand.
    // Otherwise, you will have at least 300 ms to decide your hand.
    // KumipuyoSeq will have at least 2 kumipuyos. When we know more Kumipuyo sequence,
    // it might contain more.
    virtual DropDecision think(int frameId, const CoreField&, const KumipuyoSeq&,
                               const PlayerState& me, const PlayerState& enemy, bool fast) const = 0;

    // gaze will be called when AI should gaze the enemy's field.
    // |frameId| is the frameId where the enemy has started moving his puyo.
    // His moving puyo is the front puyo of the KumipuyoSeq.
    // KumipuyoSeq has at least 2 kumipuyos. When we know more Kumipuyo sequence,
    // it might contain more.
    // Since gaze might be called in the same frame as think(), you shouldn't consume
    // much time for gaze.
    virtual void gaze(int frameId, const CoreField& enemyField, const KumipuyoSeq&);

    // Set AI's behavior.
    void setBehaviorDefensive(bool flag) { behaviorDefensive_ = flag; }
    // Set AI's behavior. If true, you can rethink next decision when the enemy has started his rensa.
    void setBehaviorRethinkAfterOpponentRensa(bool flag) { behaviorRethinkAfterOpponentRensa_ = flag; }

    // These callbacks will be called from the corresponding method.
    // i.e. onXXX() will be called from XXX().
    virtual void onGameWillBegin(const FrameRequest&) {}
    virtual void onGameHasEnded(const FrameRequest&) {}
    virtual void onDecisionRequested(const FrameRequest&) {}
    virtual void onGrounded(const FrameRequest&) {}
    virtual void onOjamaDropped(const FrameRequest&) {}
    virtual void onNext2Appeared(const FrameRequest&) {}
    virtual void onEnemyDecisionRequested(const FrameRequest&) {}
    virtual void onEnemyGrounded(const FrameRequest&) {}
    virtual void onEnemyOjamaDropped(const FrameRequest&) {}
    virtual void onEnemyNext2Appeared(const FrameRequest&) {}

    // |gameWillBegin| will be called just before a new game will begin.
    // FrameRequest might contain NEXT and NEXT2 puyos, but it's not guaranteed.
    // Please initialize your AI in this function.
    void gameWillBegin(const FrameRequest&);

    // |gameHasEnded| will be called just after a game has ended.
    void gameHasEnded(const FrameRequest&);

    void decisionRequested(const FrameRequest&);
    void grounded(const FrameRequest&);
    void ojamaDropped(const FrameRequest&);
    void next2Appeared(const FrameRequest&);

    // When enemy will start to move puyo, this callback will be called.
    void enemyDecisionRequested(const FrameRequest&);

    // When enemy's puyo is grounded, this callback will be called.
    // Enemy's rensa is automatically checked, so you don't need to do that. (Use AdditionalThoughtInfo)
    void enemyGrounded(const FrameRequest&);
    void enemyOjamaDropped(const FrameRequest&);

    // When enemy's NEXT2 has appeared, this callback will be called.
    // You can update the enemy information here.
    void enemyNext2Appeared(const FrameRequest&);

    // Should rethink just before sending next decision.
    void requestRethink() { rethinkRequested_ = true; }

    const PlayerState& myPlayerState() const { return me_; }
    const PlayerState& enemyPlayerState() const { return enemy_; }

protected:
    PlayerState* mutableMyPlayerState() { return &me_; }
    PlayerState* mutableEnemyPlayerState() { return &enemy_; }

private:
    friend class AITest;
    friend class Endless;
    friend class Solver;

    static bool isFieldInconsistent(const PlainField& ours, const PlainField& provided);
    static void mergeField(CoreField* ours, const PlainField& provided);

    KumipuyoSeq rememberedSequence(int indexFrom) const;

    std::string name_;
    ClientConnector connector_;

    bool rethinkRequested_;
    int enemyDecisionRequestFrameId_;

    PlayerState me_;
    PlayerState enemy_;

    bool behaviorDefensive_;
    bool behaviorRethinkAfterOpponentRensa_;
};

#endif
