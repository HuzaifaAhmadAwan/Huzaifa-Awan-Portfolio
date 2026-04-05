#ifndef PDBS_TYPES_H
#define PDBS_TYPES_H

#include <vector>

namespace pdbs {

/// Type alias for patterns, represented as a list of variables (by id).
using Pattern = std::vector<int>;

/// Type alias for pattern collections, represented as a list of patterns.
using PatternCollection = std::vector<Pattern>;

}

#endif
