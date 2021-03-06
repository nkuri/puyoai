#ifndef CORE_PUYO_CONTROLLER_H_
#define CORE_PUYO_CONTROLLER_H_

#include "core/key_set.h"

class CoreField;
class Decision;
class KumipuyoMovingState;
class PlainField;

class PuyoController {
public:
    static bool isReachable(const CoreField&, const Decision&);
    static bool isReachableFrom(const PlainField&, const KumipuyoMovingState&, const Decision&);

    // Finds a key stroke to move puyo from |KumipuyoMovingState| to |Decision|.
    // When there is not such a way, the returned KeySetSeq would be empty sequence.
    static KeySetSeq findKeyStroke(const CoreField&, const Decision&);
    static KeySetSeq findKeyStrokeFrom(const CoreField&, const KumipuyoMovingState&, const Decision&);

private:
    static KeySetSeq findKeyStrokeOnlineInternal(const PlainField&, const KumipuyoMovingState&, const Decision&);

    // Fast, but usable in limited situation.
    static KeySetSeq findKeyStrokeFastpath(const CoreField&, const Decision&);
    // This is faster, but might output worse key stroke.
    static KeySetSeq findKeyStrokeOnline(const PlainField&, const KumipuyoMovingState&, const Decision&);
    // This is slow, but precise.
    static KeySetSeq findKeyStrokeByDijkstra(const PlainField&, const KumipuyoMovingState&, const Decision&);
};

#endif  // CORE_PUYO_CONTROLLER_H_
