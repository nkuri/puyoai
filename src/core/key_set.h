#ifndef CORE_KEY_SET_H_
#define CORE_KEY_SET_H_

#include <bitset>
#include <cstddef>
#include <initializer_list>
#include <string>
#include <vector>

#include "core/key.h"

class KeySet {
public:
    KeySet() {}
    explicit KeySet(Key);
    KeySet(Key, Key);

    void setKey(Key, bool b = true);
    void setKey(char c);
    bool hasKey(Key) const;

    bool hasTurnKey() const { return hasKey(Key::RIGHT_TURN) || hasKey(Key::LEFT_TURN); }
    bool hasArrowKey() const { return hasKey(Key::RIGHT) || hasKey(Key::LEFT) || hasKey(Key::UP) || hasKey(Key::DOWN); }
    bool hasSomeKey() const { return keys_.any(); }
    bool hasIntersection(const KeySet& rhs) const
    {
        const KeySet& lhs = *this;
        return (lhs.keys_ & rhs.keys_).any();
    }

    std::string toString() const;
    int toInt() const { return static_cast<int>(keys_.to_ulong()); }

    friend bool operator==(const KeySet& lhs, const KeySet& rhs) { return lhs.keys_ == rhs.keys_; }
    friend bool operator!=(const KeySet& lhs, const KeySet& rhs) { return lhs.keys_ != rhs.keys_; }

private:
    std::bitset<NUM_KEYS> keys_;
};

class KeySetSeq {
public:
    KeySetSeq() {}
    explicit KeySetSeq(const std::vector<KeySet>& seq) : seq_(seq) {}
    explicit KeySetSeq(std::initializer_list<KeySet> seq) : seq_(seq) {}
    explicit KeySetSeq(const std::string&);

    bool empty() const { return seq_.empty(); }
    size_t size() const { return seq_.size(); }
    const KeySet& front() const { return seq_.front(); }
    const KeySet& back() const { return seq_.back(); }
    const KeySet& operator[](int idx) const { return seq_[idx]; }

    std::vector<KeySet>::iterator begin() { return seq_.begin(); }
    std::vector<KeySet>::const_iterator begin() const { return seq_.cbegin(); }
    std::vector<KeySet>::iterator end() { return seq_.end(); }
    std::vector<KeySet>::const_iterator end() const { return seq_.cend(); }

    void add(const KeySet& ks) { seq_.push_back(ks); }
    // TODO(mayah): This is not O(1) but O(N). However, this method won't be called much.
    // So it would be acceptable.
    void removeFront() { seq_.erase(seq_.begin()); }
    // TODO(mayah): This is O(N).
    void addFront(const KeySet& ks) { seq_.insert(seq_.begin(), ks); }

    void clear() { seq_.clear(); }

    std::string toString() const;

private:
    std::vector<KeySet> seq_;
};

#endif
