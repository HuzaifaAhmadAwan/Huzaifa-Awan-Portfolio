#ifndef HEURISTICS_MAX_HEURISTIC_H
#define HEURISTICS_MAX_HEURISTIC_H

#include <memory>

class ClassicalPlanningTask;
class Heuristic;
class BestSupporterFunction;

namespace max_heuristic {

/**
 * @brief Creates the \f$\hmax\f$ heuristic for a given planning task.
 *
 * The \f$\hmax\f$ heuristic approximates the cost of a state by the
 * cost of the most expensive goal fact in the delete-relaxed problem that has
 * not been achieved yet.
 *
 * For a planning task
 * \f$ \task = (\varspace, \operators, \initstate, \taskgoal)\f$, the
 * \f$\hmax\f$ heuristic is formally defined as
 * \f$\hmax(s) := \hmax(s, \taskgoal)\f$, where
 * \f$\hmax(s, g)\f$ is the point-wise greatest function satisfying
 *
 * \f[
 * \hmax(s, g) =
 * \begin{cases}
 *   0 & s \subseteq g, \\
 *   \inf_{a \in A, g' \in \mathit{add}_a}
 *   c(a) + \hmax(s, \mathit{pre}_a) & g = \{g'\},\\
 *   \max_{g' \in g} \hmax(s, \{g'\}) & |g| > 1.
 * \end{cases}
 * \f]
 *
 * @ingroup heuristics
 */
std::unique_ptr<Heuristic> create_hmax_heuristic(
    std::shared_ptr<ClassicalPlanningTask> task);

/**
 * @brief Creates the best support function for \f$\hmax\f$ for a given planning
 * task.
 *
 * With respect to the initial state \f$s\f$, this best supporter function
 * returns some operator
 * \f[
 * a' \in \argmin_{a \in \ops, g \in \mathit{add}_a}
 * c(a) + \hmax(s, \mathit{pre}_a)
 * \f]
 * for the fact \f$g\f$, if such an operator exists.
 * Otherwise, `std::nullopt` is returned.
 *
 * @ingroup heuristics
 */
std::unique_ptr<BestSupporterFunction> create_hmax_best_supporter_function(
    std::shared_ptr<ClassicalPlanningTask> task);

} // namespace max_heuristic

#endif
