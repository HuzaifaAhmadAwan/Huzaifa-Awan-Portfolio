#ifndef HEURISTICS_FF_HEURISTIC_H
#define HEURISTICS_FF_HEURISTIC_H

#include "downward/heuristics/delete_relaxed_plan_heuristic.h"

class BestSupporterFunction;
class ClassicalPlanningTask;

namespace ff_heuristic {

/**
 * @brief Creates the \f$\hff\f$ heuristic for the given best supporter
 * function and planning task.
 *
 * The \f$\hff\f$ heuristic extracts a delete-relaxed plan using a best
 * supporter function and returns the cost of this delete-relaxed plan.
 * The delete-relaxed plan that is returned is not always unique and may be
 * affected by tie-breaking of best supporters.
 * Any delete-relaxed plan that is extractable from the best supporter function
 * under any tie-breaking rule is valid.
 * In particular, non-deterministic implementations are allowed, i.e., it is
 * valid to return different delete-relaxed plans for the same state for
 * seperate calls.
 *
 * @ingroup heuristics
 */
std::unique_ptr<DeleteRelaxedPlanHeuristic> create_hff_heuristic(
    std::shared_ptr<ClassicalPlanningTask> task,
    std::shared_ptr<BestSupporterFunction> best_supporter_function);

} // namespace ff_heuristic

#endif
