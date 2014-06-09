#include "core/ctrl.h"

#include "core/decision.h"
#include "core/kumipuyo.h"
#include "core/plain_field.h"

bool Ctrl::isReachable(const PlainField& field, const Decision& decision)
{
    return isReachableOnline(field, KumipuyoPos(decision.x, 1, decision.r), KumipuyoPos::InitialPos());
}

bool Ctrl::isReachableOnline(const PlainField& field, const KumipuyoPos& goal, const KumipuyoPos& start)
{
    std::vector<KeyTuple> ret;
    return getControlOnline(field, goal, start, &ret);
}

bool Ctrl::isQuickturn(const PlainField& field, const KumipuyoPos& k)
{
    // assume that k.r == 0 or 2
    return (field.get(k.x - 1, k.y) != PuyoColor::EMPTY && field.get(k.x + 1, k.y) != PuyoColor::EMPTY);
}

bool Ctrl::getControl(const PlainField& field, const Decision& decision, std::vector<KeyTuple>* ret)
{
    ret->clear();

    if (!isReachable(field, decision)) {
        return false;
    }
    int x = decision.x;
    int r = decision.r;

    switch (r) {
    case 0:
        moveHorizontally(x - 3, ret);
        break;

    case 1:
        add(KEY_RIGHT_TURN, ret);
        moveHorizontally(x - 3, ret);
        break;

    case 2:
        moveHorizontally(x - 3, ret);
        if (x < 3) {
            add(KEY_RIGHT_TURN, ret);
            add(KEY_RIGHT_TURN, ret);
        } else if (x > 3) {
            add(KEY_LEFT_TURN, ret);
            add(KEY_LEFT_TURN, ret);
        } else {
            if (field.get(4, 12) != PuyoColor::EMPTY) {
                if (field.get(2, 12) != PuyoColor::EMPTY) {
                    // fever's quick turn
                    add(KEY_RIGHT_TURN, ret);
                    add(KEY_RIGHT_TURN, ret);
                } else {
                    add(KEY_LEFT_TURN, ret);
                    add(KEY_LEFT_TURN, ret);
                }
            } else {
                add(KEY_RIGHT_TURN, ret);
                add(KEY_RIGHT_TURN, ret);
            }
        }
        break;

    case 3:
        add(KEY_LEFT_TURN, ret);
        moveHorizontally(x - 3, ret);
        break;

    default:
        LOG(FATAL) << r;
    }

    add(KEY_DOWN, ret);

    return true;
}

// returns null if not reachable
bool Ctrl::getControlOnline(const PlainField& field, const KumipuyoPos& goal, const KumipuyoPos& start, std::vector<KeyTuple>* ret)
{
    KumipuyoPos current = start;

    ret->clear();
    while (true) {
        if (goal.x == current.x && goal.r == current.r) {
            break;
        }

        // for simpicity, direct child-puyo upwards
        // TODO(yamaguchi): eliminate unnecessary moves
        if (current.r == 1) {
            add(KEY_LEFT_TURN, ret);
            current.r = 0;
        } else if (current.r == 3) {
            add(KEY_RIGHT_TURN, ret);
            current.r = 0;
        } else if (current.r == 2) {
            if (isQuickturn(field, current)) {
                // do quick turn
                add(KEY_RIGHT_TURN, ret);
                add(KEY_RIGHT_TURN, ret);
                current.y++;
            } else {
                if (field.get(current.x - 1, current.y) != PuyoColor::EMPTY) {
                    add(KEY_LEFT_TURN, ret);
                    add(KEY_LEFT_TURN, ret);
                } else {
                    add(KEY_RIGHT_TURN, ret);
                    add(KEY_RIGHT_TURN, ret);
                }
            }
            current.r = 0;
        }
        if (goal.x == current.x) {
            switch(goal.r) {
            case 0:
                break;
            case 1:
                if (field.get(current.x + 1, current.y) != PuyoColor::EMPTY) {
                    if (field.get(current.x + 1, current.y + 1) != PuyoColor::EMPTY ||
                        field.get(current.x, current.y - 1) == PuyoColor::EMPTY) {
                        return false;
                    }
                    // turn inversely to avoid kicking wall
                    add(KEY_LEFT_TURN, ret);
                    add(KEY_LEFT_TURN, ret);
                    add(KEY_LEFT_TURN, ret);
                } else {
                    add(KEY_RIGHT_TURN, ret);
                }
                break;
            case 3:
                if (field.get(current.x - 1, current.y) != PuyoColor::EMPTY) {
                    if (field.get(current.x - 1, current.y + 1) != PuyoColor::EMPTY ||
                        field.get(current.x, current.y - 1) == PuyoColor::EMPTY) {
                        return false;
                    }
                    add(KEY_RIGHT_TURN, ret);
                    add(KEY_RIGHT_TURN, ret);
                    add(KEY_RIGHT_TURN, ret);
                } else {
                    add(KEY_LEFT_TURN, ret);
                }
                break;
            case 2:
                if (field.get(current.x - 1, current.y) != PuyoColor::EMPTY) {
                    add(KEY_RIGHT_TURN, ret);
                    add(KEY_RIGHT_TURN, ret);
                } else {
                    add(KEY_LEFT_TURN, ret);
                    add(KEY_LEFT_TURN, ret);
                }
                break;
            }
            break;
        }

        // direction to move horizontally
        if (goal.x > current.x) {
            // move to right
            if (field.get(current.x + 1, current.y) == PuyoColor::EMPTY) {
                add(KEY_RIGHT, ret);
                current.x++;
            } else {  // hits a wall
                // climb if possible
                /*
                  aBb
                  .A@
                  .@@.
                */
                // pivot puyo cannot go up anymore
                if (current.y >= 13) return false;
                // check "b"
                if (field.get(current.x + 1, current.y + 1) != PuyoColor::EMPTY) {
                    return false;
                }
                if (field.get(current.x, current.y - 1) != PuyoColor::EMPTY || isQuickturn(field, current)) {
                    // can climb by kicking the ground or quick turn. In either case,
                    // kumi-puyo is never moved because right side is blocked

                    add(KEY_LEFT_TURN, ret);
                    add(KEY_LEFT_TURN, ret);
                    current.y++;
                    if (!field.get(current.x - 1, current.y + 1)) {
                        add(KEY_RIGHT_TURN, ret);
                        add(KEY_RIGHT, ret);
                    } else {
                        // if "a" in the figure is filled, kicks wall. we can omit right key.
                        add(KEY_RIGHT_TURN, ret);
                    }
                    add(KEY_RIGHT_TURN, ret);
                    current.x++;
                } else {
                    return false;
                }
            }
        } else {
            // move to left
            if (!field.get(current.x - 1, current.y)) {
                add(KEY_LEFT, ret);
                current.x--;
            } else {  // hits a wall
                // climb if possible
                /*
                  bBa
                  @A.
                  @@@.
                */
                // pivot puyo cannot go up anymore
                if (current.y >= 13) return false;
                // check "b"
                if (field.get(current.x - 1, current.y + 1) != PuyoColor::EMPTY) {
                    return false;
                }
                if (field.get(current.x, current.y - 1) != PuyoColor::EMPTY || isQuickturn(field, current)) {
                    // can climb by kicking the ground or quick turn. In either case,
                    // kumi-puyo is never moved because left side is blocked
                    add(KEY_RIGHT_TURN, ret);
                    add(KEY_RIGHT_TURN, ret);
                    current.y++;
                    if (!field.get(current.x + 1, current.y)) {
                        add(KEY_LEFT_TURN, ret);
                        add(KEY_LEFT, ret);
                    } else {
                        // if "a" in the figure is filled, kicks wall. we can omit left key.
                        add(KEY_LEFT_TURN, ret);
                    }
                    add(KEY_LEFT_TURN, ret);
                    current.x--;
                } else {
                    return false;
                }
            }
        }
    }
    add(KEY_DOWN, ret);
    //LOG(INFO) << buttonsDebugString();
    return true;
}

std::string Ctrl::buttonsDebugString(const std::vector<KeyTuple>& ret)
{
    char cmds[] = " ^>v<AB";
    // caution: this string is used by test cases.
    std::string out;
    for (int i = 0; i < int(ret.size()); i++) {
        if (i != 0) {
            out += ',';
        }
        if (ret[i].b1) {
            out += cmds[ret[i].b1];
        }
        if (ret[i].b2) {
            out += cmds[ret[i].b2];
        }
    }
    return out;
}

void Ctrl::add(Key b, std::vector<KeyTuple>* ret)
{
    ret->push_back(KeyTuple(b, KEY_NONE));
}

void Ctrl::add2(Key b1, Key b2, std::vector<KeyTuple>* ret)
{
    ret->push_back(KeyTuple(b1, b2));
}

void Ctrl::moveHorizontally(int x, std::vector<KeyTuple>* ret)
{
    if (x < 0) {
        for (int i = 0; i < -x; i++) {
            add(KEY_LEFT, ret);
        }
    } else if (x > 0) {
        for (int i = 0; i < x; i++) {
            add(KEY_RIGHT, ret);
        }
    }
}