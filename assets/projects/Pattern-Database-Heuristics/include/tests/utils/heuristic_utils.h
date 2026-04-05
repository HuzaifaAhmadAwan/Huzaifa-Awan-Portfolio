#ifndef HEURISTIC_UTILS_H
#define HEURISTIC_UTILS_H

class ClassicalPlanningTask;
class Heuristic;

namespace tests {

/**
 * @brief Computes the estimate of the given heuristic for the given task's
 * initial state.
 *
 * @ingroup classical_planning_utils
 */
int get_initial_state_estimate(
    const ClassicalPlanningTask& task,
    Heuristic& heuristic);

} // namespace tests

#endif
