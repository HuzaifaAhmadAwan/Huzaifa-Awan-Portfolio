#ifndef HEURISTICS_GOAL_COUNT_HEURISTIC_H
#define HEURISTICS_GOAL_COUNT_HEURISTIC_H

#include <memory>

class ClassicalPlanningTask;
class Heuristic;

namespace goal_count_heuristic {

/**
 * @brief Creates the goal-counting heuristic \f$h^{\text{GC}}\f$ for a given
 * planning task.
 *
 * This heuristic returns the number of missing goal facts in the current state.
 *
 * Formally, for a planning task \f$ \Pi = (V, A, I, G)\f$, the blind heuristic
 * \f$h^{\text{GC}}\f$ is defined by
 *
 * \f[
 * h^{\text{GC}}(s) = |\{v \in V[G] \mid G(v) \neq s(v) \}|
 * \f]
 *
 * @ingroup heuristics
 */
std::unique_ptr<Heuristic> create_goal_count_heuristic(
    std::shared_ptr<ClassicalPlanningTask> task);

} // namespace goal_count_heuristic

#endif
