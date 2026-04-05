#ifndef PDB_UTILS_H
#define PDB_UTILS_H

#include "downward/pdbs/types.h"

#include "downward/task_proxy.h"

namespace pdbs {
class PDBHeuristic;
class CliquesHeuristic;
} // namespace pdbs

namespace tests {

/**
 * @brief Implements a greedy pattern generation algorithm.
 *
 * Obtains a pattern for the input task that is not larger than the specified
 * limit by running the following greedy algorithm:
 *
 * Sort the variables of the task with respect to the following lexicographic
 * order:
 * 1. Goal variables before non-goal variables
 * 2. Variables with smaller domain size before variables with larger domain
 *    size
 *
 * In this order, add variables greedily to the pattern until the number of
 * abstract states in the corresponding projection's transition system would
 * exceed the specified limit or no variables are left, then return the pattern.
 *
 * @ingroup pdb_tests
 */
pdbs::Pattern
get_greedy_pattern(
    const ClassicalPlanningTask& task, int abstract_state_limit = 10000);

/**
 * @brief Generates the pattern collection consisting of all atomic goal
 * variable patterns.
 *
 * An atomic pattern consists of a single variable.
 * The function returns the pattern collection
 * \f$\{ \{v\} | v is a goal variable \}\f$
 * consisting of all atomic patterns with a single goal variable.
 *
 * @ingroup pdb_tests
 */
pdbs::PatternCollection get_atomic_patterns(const ClassicalPlanningTask& task);

}

#endif // PDB_UTILS_H
