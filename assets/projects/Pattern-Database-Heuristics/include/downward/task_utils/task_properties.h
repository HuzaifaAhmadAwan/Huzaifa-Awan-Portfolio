#ifndef TASK_UTILS_TASK_PROPERTIES_H
#define TASK_UTILS_TASK_PROPERTIES_H

#include "downward/concepts.h"
#include "downward/per_task_information.h"

#include "downward/algorithms/int_packer.h"

namespace utils {
class LogProxy;
}

class AbstractOperatorProxy;
class AbstractPlanningTask;
class ClassicalPlanningTask;
class GoalProxy;
class State;

namespace task_properties {

/**
 * @brief Checks whether an operator is applicable in a state.
 *
 * @ingroup utilities
 */
extern bool is_applicable(const AbstractOperatorProxy& op, const State& state);

/**
 * @brief Checks whether a state is a goal state of a planning task.
 *
 * @ingroup utilities
 */
extern bool
is_goal_state(const AbstractPlanningTask& task, const State& state);

/*
  Return true iff all operators have cost 1.

  Runtime: O(n), where n is the number of operators.
*/
extern bool is_unit_cost(const ClassicalPlanningTask& task);

extern std::vector<int>
get_operator_costs(const ClassicalPlanningTask& task);

extern double
get_average_operator_cost(const ClassicalPlanningTask& task);

extern int get_min_operator_cost(const ClassicalPlanningTask& task);

/*
  Return the number of facts of the task.
  Runtime: O(n), where n is the number of state variables.
*/
extern int get_num_facts(const AbstractPlanningTask& task);

/*
  Return the total number of effects of the task.
  Runtime: O(n), where n is the number of operators.
*/
extern int get_num_total_effects(const ClassicalPlanningTask& task);

std::vector<FactPair> get_fact_pairs(const InputRange<FactProxy> auto& facts)
{
    std::vector<FactPair> fact_pairs;
    fact_pairs.reserve(facts.size());
    for (FactProxy fact : facts) {
        fact_pairs.push_back(fact.get_pair());
    }
    return fact_pairs;
}

extern void print_variable_statistics(
    const AbstractPlanningTask& task,
    utils::LogProxy& log);

extern void
dump_pddl(const AbstractPlanningTask& task, const State& state);
extern void
dump_fdr(const AbstractPlanningTask& task, const State& state);

extern void dump_goals(const GoalProxy& goals);
extern void dump_task(const ClassicalPlanningTask& task);

extern PerTaskInformation<int_packer::IntPacker> g_state_packers;

} // namespace task_properties

#endif
