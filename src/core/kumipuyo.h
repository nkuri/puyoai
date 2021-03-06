#ifndef CORE_KUMIPUYO_H_
#define CORE_KUMIPUYO_H_

#include <ostream>
#include <string>

#include "core/puyo_color.h"

class Kumipuyo {
public:
    constexpr Kumipuyo() : axis(PuyoColor::EMPTY), child(PuyoColor::EMPTY) {}
    constexpr Kumipuyo(PuyoColor axis, PuyoColor child) : axis(axis), child(child) {}

    std::string toString() const;

    bool isValid() const { return isNormalColor(axis) && isNormalColor(child); }
    constexpr Kumipuyo reverse() const { return Kumipuyo(child, axis); }
    bool isRep() const { return axis == child; }

    friend bool operator==(const Kumipuyo& lhs, const Kumipuyo& rhs) { return lhs.axis == rhs.axis && lhs.child == rhs.child; }
    friend bool operator!=(const Kumipuyo& lhs, const Kumipuyo& rhs) { return !(lhs == rhs); }

    friend std::ostream& operator<<(std::ostream& os, const Kumipuyo& kp) { return os << kp.toString(); }
public:
    PuyoColor axis;
    PuyoColor child;
};

#endif
