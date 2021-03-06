#ifndef CPU_MAYAH_PATTERN_BOOK_H_
#define CPU_MAYAH_PATTERN_BOOK_H_

#include <map>
#include <string>
#include <vector>

#include <toml/toml.h>

#include "base/noncopyable.h"
#include "core/algorithm/field_pattern.h"
#include "core/algorithm/rensa_detector.h"
#include "core/column_puyo_list.h"
#include "core/core_field.h"
#include "core/position.h"

class PatternBookField {
public:
    // If ignitionColumn is 0, we ignore ignitionColumn when detecting rensa.
    PatternBookField(const std::string& field, const std::string& name, int ignitionColumn, double score);

    const std::string& name() const { return name_; }
    double score() const { return score_; }
    int ignitionColumn() const { return ignitionColumn_; }
    const std::vector<Position>& ignitionPositions() const { return ignitionPositions_; }

    const FieldPattern& pattern() const { return pattern_; }
    FieldPattern* mutablePattern() { return &pattern_; }

    int numVariables() const { return pattern_.numVariables(); }

    bool isMatchable(const CoreField& cf) const { return pattern_.isMatchable(cf); }
    ComplementResult complement(const CoreField& cf,
                                int numAllowingFillingUnusedVariables,
                                ColumnPuyoList* cpl) const
    {
        return pattern_.complement(cf, numAllowingFillingUnusedVariables, cpl);
    }

    PatternBookField mirror() const
    {
        int mirroredIgnitionColumn = ignitionColumn() == 0 ? 0 : 7 - ignitionColumn();
        return PatternBookField(pattern_.mirror(), name(), mirroredIgnitionColumn, score());
    }

private:
    PatternBookField(const FieldPattern&, const std::string& name, int ignitionColumn, double score);

    FieldPattern pattern_;
    std::string name_;
    int ignitionColumn_;
    double score_;
    std::vector<Position> ignitionPositions_;
};

class PatternBook : noncopyable {
public:
    typedef std::multimap<std::vector<Position>, int> IndexMap;
    typedef IndexMap::const_iterator IndexIterator;

    bool load(const std::string& filename);
    bool loadFromString(const std::string&);
    bool loadFromValue(const toml::Value&);

    // Finds the PatternBookField from the positions where puyos are erased at the first chain.
    // Multiple PatternBookField might be found, so begin-iterator and end-iterator will be
    // returned. If no such PatternBookField is found, begin-iterator and end-iterator are the same.
    // Note that ignitionPositions must be sorted.
    std::pair<IndexIterator, IndexIterator> find(const std::vector<Position>& ignitionPositions) const;

    size_t size() const { return fields_.size(); }
    const PatternBookField& patternBookField(int i) const { return fields_[i]; }

private:
    std::vector<PatternBookField> fields_;
    IndexMap index_;
};

#endif
