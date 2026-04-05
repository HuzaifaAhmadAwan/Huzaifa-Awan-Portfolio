#ifndef TASK_UTILS_H
#define TASK_UTILS_H

#include "probfd/task_proxy.h"

#include <memory>

class ClassicalPlanningTask;

namespace probfd {
class ProbabilisticPlanningTask;
}

namespace tests {
class ClassicalPlanningDomain;
}

namespace tests {

/**
 * @brief Creates a planning task with variables and operators given by a
 * planning and a fixed initial state and goal.
 *
 * @ingroup classical_planning_utils
 */
std::shared_ptr<ClassicalPlanningTask> create_task_from_domain(
    const ClassicalPlanningDomain& domain,
    const std::vector<FactPair>& initial_state,
    std::vector<FactPair> goal);

/**
 * @brief Turns a classical planning task into a probabilistic planning task.
 *
 * @ingroup rl_tests
 */
std::unique_ptr<probfd::ProbabilisticPlanningTask>
to_probabilistic_task(std::shared_ptr<ClassicalPlanningTask> deterministic_task);

}

#endif
