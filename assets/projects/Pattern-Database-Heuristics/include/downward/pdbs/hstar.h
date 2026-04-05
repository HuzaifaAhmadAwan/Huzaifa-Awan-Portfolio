
#ifndef DOWNWARD_PDBS_HSTAR_H
#define DOWNWARD_PDBS_HSTAR_H

#include <vector>

class ClassicalPlanningTask;

namespace pdbs {

class StateEnumerator;

/**
 * @brief Computes \f$\hstar\f$ for the given planning task.
 *
 * The output is a list of \f$\hstar\f$ values, where entry \f$i\f$ corresponds
 * to the state with this index according to the passed StateEnumerator.
 */
extern std::vector<int> compute_hstar_table(
    const ClassicalPlanningTask& task,
    const StateEnumerator& enumerator);

} // namespace pdbs

#endif
