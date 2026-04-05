#ifndef FF_UTILS_H
#define FF_UTILS_H

#include "downward/heuristics/delete_relaxed_plan_heuristic.h"

#include <vector>

class BestSupporterFunction;
class ClassicalPlanningTask;
class OperatorID;

namespace tests {

/**
 * @brief Invokes the compute_heuristic_and_relaxed_plan method of the given FF
 * heuristic on the initial state of given planning task and returns the
 * result.
 *
 * @ingroup ff_tests
 */
std::optional<DeleteRelaxedPlan> get_relaxed_plan(
    const ClassicalPlanningTask& task,
    DeleteRelaxedPlanHeuristic& heuristic);

/**
 * @brief Checks if the given operator sequence constitutes a relaxed plan for
 * the given planning task.
 *
 * @ingroup ff_tests
 */
bool verify_relaxed_plan(
    const ClassicalPlanningTask& task,
    const std::vector<OperatorID>& relaxed_plan,
    bool reject_superfluous_operators = false);

/**
 * @brief Reads a best supporter function for the given task from the file with
 * given filename in the `resources/best_supporter_functions` directory.
 *
 * @see dump_best_supporters_to_file
 *
 * @ingroup ff_tests
 */
std::shared_ptr<BestSupporterFunction> read_best_supporter_function_from_file(
    const ClassicalPlanningTask& task,
    const std::string& filename);

/**
 * @brief Dumps a best supporter function for the given task to a file with
 * given filename in the `resources/best_supporter_functions` directory.
 *
 * @see read_best_supporter_function_from_file
 *
 * @ingroup ff_tests
 */
void dump_best_supporters_to_file(
    const ClassicalPlanningTask& task,
    BestSupporterFunction& bsf,
    const std::string& task_filename);

} // namespace tests

#endif
