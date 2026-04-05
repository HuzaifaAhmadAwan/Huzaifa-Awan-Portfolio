#ifndef PROBFD_TASK_UTILS_TASK_PROPERTIES_H
#define PROBFD_TASK_UTILS_TASK_PROPERTIES_H

#include "probfd/task_proxy.h"

#include "probfd/value_type.h"

#include "downward/operator_cost.h"
#include "downward/task_proxy.h"

#include <vector>

// Forward Declarations
class ClassicalPlanningTask;

namespace probfd {
class ProbabilisticPlanningTask;
}

namespace probfd::task_properties {

bool is_applicable(const ProbabilisticOperatorProxy& op, const State& state);

value_t get_adjusted_action_cost(
    const ProbabilisticOperatorProxy& op,
    OperatorCost cost_type,
    bool is_unit_cost);

/*
  Return true iff all operators have cost 1.0 (exact fp equality).

  Runtime: O(n), where n is the number of probabilistic operators.
*/
extern bool is_unit_cost(const ProbabilisticPlanningTask& task);

std::vector<value_t>
get_operator_costs(const ProbabilisticPlanningTask& task);
extern value_t
get_average_operator_cost(const ProbabilisticPlanningTask& task);
extern value_t
get_min_operator_cost(const ProbabilisticPlanningTask& task);

/*
  Return the total number of effects of the task, including the
  effects of axioms.
  Runtime: O(m + n), where m is the number of axioms and n is the total
  number of operator effects.
*/
extern int get_num_total_effects(const ProbabilisticPlanningTask& task);

extern void
dump_probabilistic_task(const ProbabilisticPlanningTask& task);

} // namespace probfd::task_properties

#endif
