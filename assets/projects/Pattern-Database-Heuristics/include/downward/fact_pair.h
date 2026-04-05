#ifndef FACT_PAIR_H
#define FACT_PAIR_H

#include "downward/utils/hash.h"

#include <compare>
#include <iosfwd>

/**
 * @brief A pair of variable index and domain value index, identifiying a fact
 * \f$(v, d)\f$.
 *
 * @ingroup facts
 */
struct FactPair {
    int var;   ///< The unique variable index of this fact's variable \f$v\f$.
    int value; ///< The unique value index of this fact's domain value
               ///< \f$d \in \domain(v)\f$ of the variable \f$v\f$.

    /// Constructs a FactPair from a variable index and a domain value index.
    FactPair(int var, int value);

    /// Compares two FactPair objects lexicographically, first by variable
    /// index, then by domain value index.
    friend auto operator<=>(const FactPair&, const FactPair&) = default;

    /// This special object represents "no such fact". E.g., functions
    /// that search a fact can return "no_fact" when no matching fact is
    /// found. Compares equal to itself and unequal to any actual fact.
    static const FactPair no_fact;
};

/// Prints a FactPair to an output stream.
std::ostream& operator<<(std::ostream& os, const FactPair& fact_pair);

namespace utils {
// Internal: Hash map support
void feed(HashState& hash_state, const FactPair& fact);
} // namespace utils

#endif
