#ifndef HEURISTICS_BLIND_SEARCH_HEURISTIC_H
#define HEURISTICS_BLIND_SEARCH_HEURISTIC_H

#include <memory>

class ClassicalPlanningTask;
class Heuristic;

namespace blind_search_heuristic {

/**
 * @brief Creates the blind heuristic \f$h^{0}\f$ for a given planning task.
 *
 * This blind heuristic returns the value 0 for goal states and the minimum
 * action cost for non-goal states.
 *
 * Formally, for a planning task \f$ \Pi = (V, A, I, G)\f$, the blind heuristic
 * \f$\hblind\f$ is defined by
 *
 * \f[
 * \hblind(s) =
 * \begin{cases}
 *   0 & \taskgoal \subseteq s \\
 *   \inf_{a \in A} \cost(a) & \text{otherwise}
 * \end{cases}
 * \f]
 *
 * Here, \f$\inf\f$ is the infimum, i.e. the minimum for non-empty sets and
 * \f$\infty\f$ otherwise.
 *
 * @ingroup heuristics
 */
std::unique_ptr<Heuristic> create_blind_heuristic(
    std::shared_ptr<ClassicalPlanningTask> task);

} // namespace blind_search_heuristic

#endif
