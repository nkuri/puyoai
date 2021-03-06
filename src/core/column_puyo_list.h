#ifndef CORE_COLUMN_PUYO_LIST_H_
#define CORE_COLUMN_PUYO_LIST_H_

#include <array>
#include <iterator>
#include <string>

#include <glog/logging.h>

#include "column_puyo.h"
#include "core/puyo_color.h"

class ColumnPuyoList {
public:
    ColumnPuyoList() : size_{} {}

    int sizeOn(int x) const
    {
        DCHECK(1 <= x && x <= 6);
        return size_[x-1];
    }

    PuyoColor get(int x, int i) const
    {
        DCHECK(1 <= x && x <= 6);
        DCHECK(0 <= i && i <= sizeOn(x));
        return puyos_[x-1][i];
    }

    bool isEmpty() const { return size() == 0; }
    int size() const { return size_[0] + size_[1] + size_[2] + size_[3] + size_[4] + size_[5]; }
    void clear() { std::fill(size_, size_ + 6, 0); }

    bool add(const ColumnPuyo& cp) { return add(cp.x, cp.color); }
    bool add(int x, PuyoColor c)
    {
        DCHECK(1 <= x && x <= 6);
        if (MAX_SIZE <= size_[x-1])
            return false;
        puyos_[x-1][size_[x-1]++] = c;
        return true;
    }
    bool add(int x, PuyoColor c, int n)
    {
        DCHECK(1 <= x && x <= 6);
        if (MAX_SIZE < size_[x-1] + n)
            return false;
        for (int i = 0; i < n; ++i)
            puyos_[x-1][size_[x-1] + i] = c;
        size_[x-1] += n;
        return true;
    }

    // Appends |cpl|. If the result size exceeds the max size, false will be returned.
    // When false is returned, the object does not change.
    bool merge(const ColumnPuyoList& cpl)
    {
        for (int i = 0; i < 6; ++i) {
            if (MAX_SIZE < size_[i] + cpl.size_[i])
                return false;
        }

        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < cpl.size_[i]; ++j)
                puyos_[i][size_[i]++] = cpl.puyos_[i][j];
        }
        return true;
    }

    void removeTopFrom(int x)
    {
        DCHECK(1 <= x && x <= 6);
        DCHECK_GT(sizeOn(x), 0);
        size_[x-1] -= 1;
    }

    template<typename Func>
    void iterate(Func f)
    {
        for (int x = 1; x <= 6; ++x) {
            int h = sizeOn(x);
            for (int i = 0; i < h; ++i) {
                f(x, get(x, i));
            }
        }
    }

    std::string toString() const;

    friend bool operator==(const ColumnPuyoList&, const ColumnPuyoList&);

private:
    static const int MAX_SIZE = 8;

    // We don't make this std::vector due to performance reason.
    int size_[6];
    PuyoColor puyos_[6][MAX_SIZE];
};

#endif
